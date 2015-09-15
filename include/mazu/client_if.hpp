#ifndef __MAZU_CLIENT_IF_HPP__
#define __MAZU_CLIENT_IF_HPP__

#include <string>

namespace mazu { namespace client {

        class IReducerAgent {
        public:
            virtual void Generate(int epoch, void *blob, size_t length) = 0;
            virtual void NotifyOn(int epoch) = 0;
        };

        class IReducer {
        public:
            virtual bool IsStateful() = 0;
            inline virtual void OnSaveState(ostream &os) { }
            inline virtual void OnRestoreState(istream &is) { }
            
            virtual void OnCreate(IReducerAgent *agent, const std::string &key);
            virtual void OnRecieve(int epoch, void *blob, size_t length) = 0;
            virtual void OnNotify(int epoch);
        };

        class IMapperAgent {
        public:
            virtual void Generate(int epoch, const std::string &key, void *blob, size_t length) = 0;
        };

        class IMapper {
        public:
            virtual bool IsLocal() = 0;
            virtual void OnCreate(IMapperAgent *agent);
            virtual void OnRecieve(int epoch, void *blob, size_t length) = 0;
        };

        class IExternalStreamProxy {
        public:
            virtual void OnConnected(IMapperAgent *agent, int startEpoch) = 0;
            virtual void OnEpochPersisted(int epoch) = 0;
            virtual void OnReplay(int startEpoch) = 0;
        };

        class IClient {
        public:

            void create_funnel(const std::string &name, const std::string &reducer) = 0;
            void create_external_stream(IExternalStreamProxy *proxy, const std::string &param, const std::string &target) = 0;
            void create_stream(const std::string &name, const std::string &mapper, const std::string &source, const std::string &target) = 0;
        };
        
} }

#endif
