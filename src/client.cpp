#include "client.hpp"

using namespace std;
using namespace mazu::client;

void LocalClient::RegisterMapperFactory(const std::string &name, IMapperFactory *factory) {
    _mapperFactories[name] = factory;
}

void LocalClient::RegisterReducerFactory(const std::string &name, IReducerFactory *factory) {
    _reducerFactories[name] = factory;
}

void LocalClient::RegisterExternalSourceProxyFactory(const std::string &name, IExternalSourceProxyFactory *factory) {
    _externalSourceProxyFactories[name] = factory;
}

void LocalClient::CreateFunnel(const std::string &name, const std::string &reducer, const std::string &param) {
    auto funnel = new LocalFunnel();
    funnel->factory = _reducerFactories[reducer];
    _funnels[name] = funnel;
}

void LocalClient::CreateExternalSource(const std::string &name,
                                       const std::string &externalSource, const std::string &param,
                                       const std::string &target) {
    auto agent = CreateExternalSourceMapperAgent(target);
    auto es = _externalSourceProxyFactories[externalSource]->Create(agent, 0);
    _externalSources[name] = es;
}

void LocalClient::CreateStream(const std::string &name, const std::string &mapperType, const std::string &param, const std::string &source, const std::string &target) {
    auto agent = CreateMapperAgent(target);
    auto funnel = _funnels[source];
    auto mapper = _mapperFactories[mapperType]->Create(agent);
    funnel->subscriptions[name] = mapper;  
}

IMapperAgent *LocalClient::CreateMapperAgent(const std::string &target) {
    return new LocalMapperAgent(_funnels[target]);
}

IMapperAgent *LocalClient::CreateExternalSourceMapperAgent(const std::string &target) {
    return new LocalMapperAgent(_funnels[target]);
}

LocalMapperAgent::LocalMapperAgent(LocalFunnel *target) {
    _target = target;
}

void LocalMapperAgent::Send(const std::string &key, int epoch, void *blob, size_t length) {
    auto it = _target->reducers.find(key);
    IReducer *reducer;
    if (it == _target->reducers.end()) {
        IReducerAgent *agent = new LocalReducerAgent(_target, key);
        _target->reducers[key] = _target->factory->Create(agent, key);
        reducer = _target->reducers[key];
    }
    else {
        reducer = _target->reducers[key];
    }

    reducer->OnRecieve(epoch, blob, length);
}

LocalReducerAgent::LocalReducerAgent(LocalFunnel *funnel, const std::string &key) {
    _funnel = funnel;
    _key = key;
}

void LocalReducerAgent::Send(int epoch, void *blob, size_t length) {
    for (auto it : _funnel->subscriptions) {
        IMapper *mapper = it.second;
        mapper->OnRecieve(_key, epoch, blob, length);
    }
}

void LocalReducerAgent::NotifyOn(int epoch) {
}
