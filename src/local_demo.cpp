#include "naive_client.hpp"
#include <iostream>
#include <string>

using namespace std;
using namespace mazu::client;

class InversedPairMapper : public IMapper {
public:

    InversedPairMapper(IMapperAgent *agent) {
        _agent = agent;
    }

    virtual void OnRecieve(const string &key, int epoch, void *blob, size_t length) {
        string data((char *)blob, length);
    }

private:
    IMapperAgent *_agent;
};

class InversedPairMapperFactory : public IMapperFactory {
public:

    virtual IMapper *Create(const string &param, IMapperAgent *agent) {
        return new InversedPairMapper(agent);
    }
};

class InversedPairReducer : public IReducer {
public:

    InversedPairReducer(IReducerAgent *agent) {
        _agent = agent;
    }

    virtual void OnRecieve(int epoch, void *data, size_t length) {
    }

    virtual void OnNotify(int epoch) {
    }

private:
    IReducerAgent *_agent; 
};

class InversedPairReducerFactory : public IReducerFactory {
public:

    virtual IReducer *Create(const string &param, IReducerAgent *agent, const string &key) {
        return new InversedPairReducer(agent);
    }
};

class EchoReducer : public IReducer {
public:

    EchoReducer(IReducerAgent *agent) {
        _agent = agent;
    }

    virtual void OnRecieve(int epoch, void *data, size_t length) {
        _agent->Send(epoch, data, length);
    }

    virtual void OnNotify(int epoch) {
    }

private:
    IReducerAgent *_agent; 
};

class EchoReducerFactory : public IReducerFactory {
public:

    virtual IReducer *Create(const string &param, IReducerAgent *agent, const string &key) {
        return new EchoReducer(agent);
    }
};

class FileSystemSource : public IExternalProxy {
public:

    FileSystemSource(IExternalProxyAgent *agent) {
        _agent = agent;
    }

private:
    IExternalProxyAgent *_agent;
};

class FileSystemSourceFactory : public IExternalProxyFactory {
public:

    virtual IExternalProxy *Create(const string &param, IExternalProxyAgent *agent, int epoch) {
        return new FileSystemSource(agent);
    }
};

int main() {
    LocalClient client;
    
    client.RegisterReducerFactory("EchoReducer", new InversedPairReducerFactory());
    client.RegisterReducerFactory("InversedPairReducer", new InversedPairReducerFactory());
    client.RegisterMapperFactory("InversedPairMapper", new InversedPairMapperFactory());
    client.RegisterExternalProxyFactory("FileSystemSource", new FileSystemSourceFactory());

    client.CreateFunnel("Input", "EchoReducer", "");
    client.CreateFunnel("WordCount", "InversedPairReducer", "");
    client.CreateStream("Mapper", "InversedPairMapper", "", "Input", "WordCount");
    client.CreateExternalSource("FileSystemInput", "FileSystemSource", "", "Input");

    return 0;
}
