#ifndef __MAZU_INTERNAL_CLIENT_HPP__
#define __MAZU_INTERNAL_CLIENT_HPP__

#include <mazu/client_if.hpp>
#include <string>
#include <map>

namespace mazu { namespace client {

        class LocalFunnel {
        public:
            IReducerFactory *factory;
            std::string      factory_param; 
            std::map<std::string, IMapper *> subscriptions;
            std::map<std::string, IReducer *> reducers;
        };

        class LocalMapperAgent : public IMapperAgent {
        public:
            LocalMapperAgent(LocalFunnel *target);
            virtual void Send(const std::string &key, int epoch, void *blob, size_t length);
            
        private:
            LocalFunnel *_target;
        };

        class LocalReducerAgent : public IReducerAgent {
        public:
            LocalReducerAgent(LocalFunnel *funnel, const std::string &key);
            virtual void Send(int epoch, void *blob, size_t length);
            virtual void NotifyOn(int epoch);

        private:
            std::string  _key; 
            LocalFunnel *_funnel;
        };

        class LocalClient : public IClient {
        public:
            void RegisterMapperFactory(const std::string &name, IMapperFactory *factory);
            void RegisterReducerFactory(const std::string &name, IReducerFactory *factory);
            void RegisterExternalSourceProxyFactory(const std::string &name, IExternalSourceProxyFactory *factory);
            
            virtual void CreateFunnel(const std::string &name, const std::string &reducer, const std::string &param);
            virtual void CreateExternalSource(const std::string &name, const std::string &externalSource, const std::string &param, const std::string &target);
            virtual void CreateStream(const std::string &name, const std::string &mapper, const std::string &param, const std::string &source, const std::string &target);

        private:
            IMapperAgent *CreateMapperAgent(const std::string &target);
            IMapperAgent *CreateExternalSourceMapperAgent(const std::string &target);

            std::map<std::string, IMapperFactory *> _mapperFactories;
            std::map<std::string, IReducerFactory *> _reducerFactories;
            std::map<std::string, IExternalSourceProxyFactory *> _externalSourceProxyFactories;
            
            std::map<std::string, LocalFunnel *> _funnels;
            std::map<std::string, IExternalSourceProxy *> _externalSources;
        };
        
} }

#endif
