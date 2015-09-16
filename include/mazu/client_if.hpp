#ifndef __MAZU_CLIENT_IF_HPP__
#define __MAZU_CLIENT_IF_HPP__

#include <string>

namespace mazu { namespace client {

        class IReducerAgent {
        public:
            virtual const std::string &GetKey() = 0;
            virtual void Send(int epoch, void *blob, size_t length) = 0;
            virtual void NotifyOn(int epoch) = 0;
        };

        class IReducer {
        public:            
            virtual void OnRecieve(int epoch, void *blob, size_t length) = 0;
            virtual void OnNotify(int epoch) = 0;
        };

        class IReducerFactory {
        public:
            virtual IReducer *Create(const std::string &param, IReducerAgent *agent, const std::string &key) = 0;
        };

        class IMapperAgent {
        public:
            virtual void Send(const std::string &key, int epoch, void *blob, size_t length) = 0;
        };

        class IMapper {
        public:
            virtual void OnRecieve(const std::string &key, int epoch, void *blob, size_t length) = 0;
        };

        class IMapperFactory {
        public:
            virtual IMapper *Create(const std::string &param, IMapperAgent *agent) = 0;
        };

        class IExternalProxy;
        
        class IExternalProxyAgent {
        public:
            virtual void Send(const std::string &key, int epoch, void *blob, size_t length) = 0;
            virtual void OnEpochComplete(int epoch) = 0;
            virtual void OnClose(IExternalProxy *self) = 0;
        };

        class IExternalProxy {
        public:
            virtual ~IExternalProxy() { }
        };

        class IExternalProxyFactory {
        public:
            virtual IExternalProxy *Create(const std::string &param, IExternalProxyAgent *agent, int epoch) = 0; 
        };

        class IClient {
        public:
            virtual void CreateFunnel(const std::string &name, const std::string &reducer, const std::string &param) = 0;
            virtual void CreateExternalSource(const std::string &name, const std::string &externalProxy, const std::string &param, const std::string &target) = 0;
            virtual void CreateStream(const std::string &name, const std::string &mapper, const std::string &param, const std::string &source, const std::string &target) = 0;
        };
        
} }

#endif
