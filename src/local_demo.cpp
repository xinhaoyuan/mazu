#include "naive_client.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <fstream>

using namespace std;
using namespace mazu::client;

class InversedPairMapper : public IMapper {
public:

    InversedPairMapper(IMapperAgent *agent) {
        _agent = agent;
    }

    virtual void OnRecieve(const string &key, int epoch, void *blob, size_t length) {
        string data((char *)blob, length);
        int start = 0;
        while (true) {
            int pos = data.find(' ', start);
            if (pos != start) {
                string *word = new string(
                    data.substr(start,
                                pos == data.npos ?
                                data.npos : (pos - start)));
                cout << "IP Mapper get word: " << *word << endl;
                _agent->Send(*word, epoch, (void *)word->c_str(), word->length());
            }
            if (pos == data.npos) break;
            start = pos + 1;
        }
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

    virtual void OnRecieve(int epoch, void *blob, size_t length) {
        string data((char *)blob, length);
        cout << "IP Reducer [" << _agent->GetKey() << "] get data: " << data << endl;
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

    virtual void OnRecieve(int epoch, void *blob, size_t length) {
        string data((char *)blob, length);
        cout << "Echo get data: " << data << endl;
        _agent->Send(epoch, blob, length);
    }

    virtual void OnNotify(int epoch) { }

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

    FileSystemSource(IExternalProxyAgent *agent, const string &path) {
        _agent = agent;
        _path = path;
        _proxyThread = new thread(&FileSystemSource::ThreadEntry, this);        
    }

    void ThreadEntry() {
        ifstream fs(_path);
        string line;
        cout << "started reading line" << endl;
        while (getline(fs, line)) {
            cout << "Send line: " << line << endl;
            _agent->Send("singleton_key", 0, (void *)line.c_str(), line.length());
        }
        cout << "ended reading line" << endl;
        _agent->OnClose();
    }

    ~FileSystemSource() {
        _proxyThread->join();
    }

private:
    thread *_proxyThread;
    IExternalProxyAgent *_agent;
    string _path;
};

class FileSystemSourceFactory : public IExternalProxyFactory {
public:

    virtual IExternalProxy *Create(const string &param, IExternalProxyAgent *agent, int epoch) {
        return new FileSystemSource(agent, param);
    }
};

int main() {
    LocalClient client;
    
    client.RegisterReducerFactory("EchoReducer", new EchoReducerFactory());
    client.RegisterReducerFactory("InversedPairReducer", new InversedPairReducerFactory());
    client.RegisterMapperFactory("InversedPairMapper", new InversedPairMapperFactory());
    client.RegisterExternalProxyFactory("FileSystemSource", new FileSystemSourceFactory());

    client.CreateFunnel("Input", "EchoReducer", "");
    client.CreateFunnel("WordCount", "InversedPairReducer", "");
    client.CreateStream("Mapper", "InversedPairMapper", "", "Input", "WordCount");
    client.CreateExternalSource("FileSystemInput", "FileSystemSource", "test.input", "Input");

    while (1) ;
    return 0;
}
