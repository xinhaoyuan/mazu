#include "naive_client.hpp"

using namespace std;
using namespace mazu::client;

void LocalClient::RegisterMapperFactory(const std::string &name, IMapperFactory *factory) {
    _mapperFactories[name] = factory;
}

void LocalClient::RegisterReducerFactory(const std::string &name, IReducerFactory *factory) {
    _reducerFactories[name] = factory;
}

void LocalClient::RegisterExternalProxyFactory(const std::string &name, IExternalProxyFactory *factory) {
    _externalProxyFactories[name] = factory;
}

void LocalClient::CreateFunnel(const std::string &name, const std::string &reducer, const std::string &param) {
    auto funnel = new LocalFunnel();
    funnel->name = name;
    funnel->factory = _reducerFactories[reducer];
    funnel->factory_param = param;
    _funnels[name] = funnel;
}

void LocalClient::CreateExternalSource(const std::string &name,
                                       const std::string &externalProxy, const std::string &param,
                                       const std::string &target) {
    auto agent = CreateExternalProxyAgent(target);
    auto es = _externalProxyFactories[externalProxy]->Create(param, agent, 0);
    _externalProxies[name] = es;
}

void LocalClient::CreateStream(const std::string &name, const std::string &mapperType, const std::string &param, const std::string &source, const std::string &target) {
    auto agent = CreateMapperAgent(target);
    auto funnel = _funnels[source];
    auto mapper = _mapperFactories[mapperType]->Create(param, agent);
    funnel->subscriptions[name] = mapper;
}

IMapperAgent *LocalClient::CreateMapperAgent(const std::string &target) {
    return new LocalMapperAgent(_funnels[target]);
}

IExternalProxyAgent *LocalClient::CreateExternalProxyAgent(const std::string &target) {
    return new LocalExternalProxyAgent(_funnels[target]);
}

LocalMapperAgent::LocalMapperAgent(LocalFunnel *target) {
    _target = target;
}

void LocalMapperAgent::Send(const std::string &key, int epoch, void *blob, size_t length) {
    auto it = _target->reducers.find(key);
    IReducer *reducer;
    if (it == _target->reducers.end()) {
        IReducerAgent *agent = new LocalReducerAgent(_target, key);
        _target->reducers[key] = _target->factory->Create(_target->factory_param, agent, key);
        reducer = _target->reducers[key];
    }
    else {
        reducer = _target->reducers[key];
    }

    reducer->OnRecieve(epoch, blob, length);
}

LocalExternalProxyAgent::LocalExternalProxyAgent(LocalFunnel *target) {
    _target = target;
}

void LocalExternalProxyAgent::Send(const std::string &key, int epoch, void *blob, size_t length) {
    auto it = _target->reducers.find(key);
    IReducer *reducer;
    if (it == _target->reducers.end()) {
        IReducerAgent *agent = new LocalReducerAgent(_target, key);
        _target->reducers[key] = _target->factory->Create(_target->factory_param, agent, key);
        reducer = _target->reducers[key];
    }
    else {
        reducer = _target->reducers[key];
    }

    reducer->OnRecieve(epoch, blob, length);
}

void LocalExternalProxyAgent::OnClose(IExternalProxy *proxy) {
}

void LocalExternalProxyAgent::OnEpochComplete(int epoch) {
}

LocalReducerAgent::LocalReducerAgent(LocalFunnel *funnel, const std::string &key) {
    _funnel = funnel;
    _key = key;
}

const std::string &LocalReducerAgent::GetKey() {
    return _key;
}

void LocalReducerAgent::Send(int epoch, void *blob, size_t length) {
    for (auto it : _funnel->subscriptions) {
        IMapper *mapper = it.second;
        mapper->OnRecieve(_key, epoch, blob, length);
    }
}

void LocalReducerAgent::NotifyOn(int epoch) {
}
