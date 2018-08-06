//
// Created by ganz on 08/09/17.
//

#ifndef FEUCS_FEUCS_HPP
#define FEUCS_FEUCS_HPP

# include <vector>
# include <list>
# include <map>
# include <queue>
# include <typeinfo>
# include <functional>
# include <unordered_map>
# include "dloader.hpp"
# include "clock.hpp"
# include "mediator.hpp"
# include "queue.hpp"

namespace futils
{
    class   EntityManager;
    class   IEntity;

    class   IComponent
    {
    protected:
        IEntity *__entity{nullptr};
        futils::type_index _typeindex;
    public:
        virtual ~IComponent() {}
        // Friend of EntityManager - Very important - fake CRTP
        void setTypeindex(futils::type_index index) noexcept {
            _typeindex = index;
        }
        // END
        void setEntity(IEntity &ent) noexcept {
            __entity = &ent;
        }
        IEntity &getEntity() const {
            return *__entity;
        }
        futils::type_index getTypeindex() const noexcept {
            return _typeindex;
        }
    };

    class   ISystem
    {
    protected:
        std::string name{"Undefined"};
        EntityManager *entityManager{nullptr};
        Mediator *events{nullptr};
        std::function<void()> afterBuild{[](){}};
        std::function<void(EntityManager *)> afterDeath{[](EntityManager *){}};

        // It will segfault if events is not set. Be careful !
        template <typename T>
        void addReaction(std::function<void(IMediatorPacket &pkg)> fun)
        {
            events->require<T>(this, fun);
        }
    public:
        virtual ~ISystem() {
            events->erase(this);
        }
        virtual void run(float elapsed = 0) = 0;
        void provideManager(EntityManager &manager) { entityManager = &manager; }
        void provideMediator(Mediator &mediator) { events = &mediator; }
        std::string const &getName() const { return name; }
        std::function<void(EntityManager *)> getAfterDeath()
        {
            return afterDeath;
        }
        std::function<void()> getAfterBuild()
        {
            return afterBuild;
        }
    };

    class StateSystem : public ISystem
    {
    protected:
        int phase{0};
    public:
        virtual void run(float elapsed = 0) = 0;
    };

    // Event
    template <typename T>
    class ComponentAttached
    {
        void verifType() {
            static_assert(std::is_base_of<IComponent, T>::value,
                          "Cannot emit event ComponentAttached with non Component Type");
        }
    public:
        T const &compo;
        ComponentAttached(T &&compo): compo(std::forward<T>(compo)) { verifType(); }
        ComponentAttached(T const &compo): compo(compo) { verifType(); }
    };

    // Event
    template <typename T>
    class ComponentDeleted
    {
        void verifType() {
            static_assert(std::is_base_of<IComponent, T>::value,
                          "Cannot emit event ComponentAttached with non Component Type");
        }
    public:
        T const &compo;
        ComponentDeleted(T &&compo): compo(std::forward<T>(compo)) { verifType(); }
        ComponentDeleted(T const &compo): compo(compo) { verifType(); }
    };

    class   IEntity
    {
        int _id;
        futils::type_index concreteType;
        std::unordered_map<futils::type_index, IComponent *>    components;

        template                            <typename Compo>
        void                                verifIsComponent()
        {
            if (!std::is_base_of<IComponent, Compo>::value)
                throw std::logic_error(std::string(typeid(Compo).name()) + " is not a Component");
        }
    public:
        // TODO: SHOULD BE PRIVATE AND FRIEND WITH ENTITY MANAGER
        std::function<bool(IComponent &)> onExtension{[](IComponent &){return false;}};
        std::function<void(IComponent &)> onDetach{[](IComponent &){return false;}};
        std::function<void()> afterBuild{[](){}};
        std::queue<std::pair<IComponent *, std::function<void()>>> lateinitComponents;
        futils::Mediator *events{nullptr};
        EntityManager *entityManager{nullptr};
        void setConcreteType(futils::type_index t)
        {
            concreteType = t;
        }
        void setId(int id) {
            _id = id;
        }
        // END.
        futils::type_index getConcreteType() const {
            return concreteType;
        }

        IEntity() {
            this->_id = futils::UID::get();
        }
        virtual ~IEntity() {}

        template    <typename Compo, typename ...Args>
        Compo       &attach(Args ...args)
        {
            verifIsComponent<Compo>();
            if (components.find(futils::type<Compo>::index) != components.end())
                throw std::runtime_error(std::string("Cannot have same component twice (") + typeid(Compo).name() + ")!");
            auto compo = new Compo(args...);
            compo->setTypeindex(futils::type<Compo>::index);
            compo->setEntity(*this);
            this->components.insert(std::pair<futils::type_index, IComponent *>(compo->getTypeindex(), compo));
            if (onExtension(*compo) == false) {
                lateinitComponents.push(std::pair<Compo *, std::function<void()>>(compo, [this, compo](){
                    events->send<ComponentAttached<Compo>>(*compo);
                }));
            } else
                events->send<ComponentAttached<Compo>>(*compo);
            return *compo;
        };

        template <typename T>
        bool has()
        {
            static_assert(std::is_base_of<IComponent, T>::value, "Error : T is not a Component in entity->has<T>()");
            for (const auto &it: components)
            {
                if (it.first == futils::type<T>::index)
                    return true;
            }
            return false;
        }

        template <typename T>
        T &get() const
        {
            for (auto &it: components)
            {
                if (it.first == futils::type<T>::index)
                    return static_cast<T &>(*it.second);
            }
            throw std::runtime_error("Entity " + std::to_string(this->getId()) + "  does not have requested component : " + std::string(typeid(T).name()));
        };

        template <typename Compo>
        bool detach()
        {
            if (components.find(futils::type<Compo>::index) == components.end())
                return false;
            auto compo = components.at(futils::type<Compo>::index);
            events->send<ComponentDeleted<Compo>>(static_cast<const Compo &>(*compo));
            components.erase(futils::type<Compo>::index);
            onDetach(*compo);
            delete compo;
            return true;
        }

        int         getId() const { return this->_id; }
    };

    // Event
    template <typename T>
    class EntityCreated
    {
        void verifType() {
            static_assert(std::is_base_of<IEntity, T>::value,
                          "Cannot emit event EntityCreated with non Entity Type");
        }
    public:
        T const &entity;
        EntityCreated(T &&entity): entity(std::forward<T>(entity)) { verifType(); }
        EntityCreated(T const &entity): entity(entity) { verifType(); }
    };

    struct SystemDestroyed
    {
        std::string name;
    };
    struct LoadStatus
    {
        bool loaded{false};
        std::string sysName{""};
    };

    class   EntityManager
    {
        unsigned int idCount{0};
        using SystemMap = std::unordered_map<std::string, ISystem *>;
        using SystemQueue = futils::Queue<std::string>;
        using ComponentContainer = std::unordered_multimap<futils::type_index, IComponent *>;
        using DynamicLibaryContainer = std::unordered_map<std::string, futils::UP<futils::Dloader>>;
        int status{0};
        int orderIndex{0};

        SystemMap systemsMap;
        SystemQueue systemsMarkedForErase;
        ComponentContainer components;

        // Containers for system ordering.
        std::map<int, ISystem *> orderMap;
        std::unordered_map<ISystem *, int> systemOrder;

        // All entities
        int counter{0};

        // Time
        futils::Clock<float> timeKeeper;

        // Event Mediator
        futils::Mediator *events{nullptr};

        // Used for memory Management to track entities created
        futils::ISystem *currentSystem{nullptr};
        std::unordered_multimap<std::string, IEntity *> temporaryEntities;
        std::unordered_map<IEntity *, std::string> temporaryEntitiesRecords;
        std::unordered_map<IEntity *, std::string> savedEntities;

        // Extensions (ISystem)
        DynamicLibaryContainer extensions;
        // Extension SystemNames
        std::unordered_map<std::string, std::string> extensionFiles;

        template <typename T>
        void verifIsEntity()
        {
            if (!std::is_base_of<IEntity, T>::value)
                throw std::logic_error(std::string(typeid(T).name()) + " is not an Entity");
        }

        template <typename T>
        void initEntity(T &entity)
        {
            entity.setId(idCount++);
            entity.setConcreteType(futils::type<T>::index);
            entity.events = events;
            entity.entityManager = this;
            entity.onExtension = [this](IComponent &compo) {
                components.insert(std::pair<futils::type_index, IComponent *>
                                          (compo.getTypeindex(), &compo));
                return true;
            };
            entity.onDetach = [this](IComponent &compo) {
                auto range = components.equal_range(compo.getTypeindex());
                std::vector<std::pair<futils::type_index, IComponent *>> save;
                int count = 0;
                for (auto it = range.first; it != range.second; it++) {
                    count++;
                    auto &pair = *it;
                    auto tmp = pair.second;
                    if (tmp != &compo)
                        save.push_back(pair);
                }
                components.erase(compo.getTypeindex());
                for (auto savedPair: save)
                {
                    components.insert(savedPair);
                }
            };
            events->send<EntityCreated<T>>(entity);
            while (!entity.lateinitComponents.empty()) {
                auto front = entity.lateinitComponents.front();
                entity.onExtension(*front.first);
                front.second(); // Notification
                entity.lateinitComponents.pop();
            }
            entity.afterBuild();
            counter++;
            std::cout << this << ": Created " << typeid(T).name() << " with id " << entity.getId() << std::endl;
        }

        bool destroyFromSaved(IEntity &entity)
        {
            auto &container = savedEntities;
            if (container.find(&entity) == container.end())
                return false;
            std::cout << currentSystem->getName() << ": Destroyed saved entity " << entity.getId() << " created by " << container[&entity] << std::endl;
            container.erase(&entity);
            delete &entity;
            counter--;
            return true;
        }

        bool destroyFromTemporary(IEntity &entity)
        {
            if (temporaryEntitiesRecords.find(&entity) == temporaryEntitiesRecords.end())
                return false;
            const auto &system = currentSystem->getName();
            const auto &creatorSystem = temporaryEntitiesRecords[&entity];
            auto range = temporaryEntities.equal_range(creatorSystem);
            for (auto it = range.first; it != range.second; it++) {
                if (it->second == &entity)
                {
                    temporaryEntities.erase(it);
                    break ;
                }
            }
            std::cerr << system << ": Destroyed temporary entity " << entity.getId() << " created by " << creatorSystem << std::endl;
            temporaryEntitiesRecords.erase(&entity);
            delete &entity;
            counter--;
            return true;
        }

        void initSystem(ISystem &system)
        {
            // TODO : Smart Pointer !!
            system.provideManager(*this);
            system.provideMediator(*events);
            auto afterBuild = system.getAfterBuild();
            auto *save = currentSystem;
            currentSystem = &system;
            afterBuild();
            currentSystem = save;
            events->send<std::string>("[" + system.getName() + "] loaded.");
            this->systemsMap.insert(std::pair<std::string, ISystem *>(system.getName(), &system));
            orderMap[orderIndex] = &system;
            systemOrder[&system] = orderIndex;
            orderIndex++;
        }
    public:
        EntityManager() {
            timeKeeper.start();
        }

        template    <typename T, typename ...Args>
        T &smartCreate(Args ...args)
        {
            verifIsEntity<T>();
            auto entity = new T(args...);
            initEntity(*entity);
            const auto &name = currentSystem->getName();
            temporaryEntities.insert(std::pair<std::string, IEntity *>(name, entity));
            temporaryEntitiesRecords[entity] = name;
            std::cout << "[" << name << "] created " << typeid(T).name() << " with id " << entity->getId() << std::endl;
            return *entity;
        }

        template <typename T, typename ...Args>
        T &create(Args ...args)
        {
            verifIsEntity<T>();
            auto entity = new T(args...);
            initEntity(*entity);
            savedEntities.insert(std::pair<IEntity *, std::string>(entity, currentSystem->getName()));
            return *entity;
        };

        bool destroy(IEntity &entity)
        {
            if (!destroyFromSaved(entity))
                return destroyFromTemporary(entity);
            return true;
        }

        template    <typename System, typename ...Args>
        void addSystem(Args ...args)
        {
            if (!std::is_base_of<ISystem, System>::value)
                throw std::logic_error(std::string(typeid(System).name()) + " is not a System");
            auto system = new System(args...);
            if (systemsMap.find(system->getName()) == systemsMap.end())
                initSystem(*system);
            else
                events->send<std::string>("[" + system->getName() + "] already loaded.");
        }

        template <typename ...Args>
        LoadStatus loadSystem(std::string const &path, Args ...args)
        {
            LoadStatus ret;
            if (extensions.find(path) != extensions.end()) {
                std::cerr << path << " already loaded." << std::endl;
                return ret;
            }
            extensions[path] = std::make_unique<Dloader>(Dloader(path));
            auto system = extensions[path]->build<ISystem>(args...);
            std::cout << "System " << system->getName() << " loaded from path " << path << std::endl;
            initSystem(*system);
            ret.loaded = true;
            ret.sysName = system->getName();
            extensionFiles[ret.sysName] = path;
            return ret;
        };

        void removeSystem(std::string const &systemName)
        {
            if (systemsMarkedForErase.find(systemName))
                return ;
            systemsMarkedForErase.push(systemName);
        }

        template <typename T>
        std::vector<T *> get()
        {
            static_assert(std::is_base_of<IComponent, T>::value, "Error : T is not a Component");
            std::vector<T *> res;
            try {
                auto range = components.equal_range(futils::type<T>::index);
                if (range.first == range.second) {
                    return {};
                }
                for (auto it = range.first; it != range.second; it++) {
                    res.push_back(static_cast<T *>(it->second));
                }
            } catch (std::exception const &e) {
                std::cerr << e.what() << std::endl;
            }
            return res;
        };

        void provideMediator(Mediator &mediator) {
            events = &mediator;
        }

        bool        isFine() const
        {
            return this->status == 0;
        }

        int getNumberOfSystems() const
        {
            return systemsMap.size();
        }

        void cleanSystems()
        {
            while (!systemsMarkedForErase.empty()) {
                auto name = systemsMarkedForErase.front();
                auto system = systemsMap.at(name);
                events->erase(system);
                systemsMap.erase(name);
                orderMap.erase(systemOrder[system]);
                systemOrder.erase(system);
                auto afterDeath = system->getAfterDeath();
                // Delete all temporary entities created by this system.
                auto range = temporaryEntities.equal_range(name);
                int entitiesDeleted = 0;
                for (auto it = range.first; it != range.second; it++) {
                    if (temporaryEntitiesRecords.find(it->second) == temporaryEntitiesRecords.end())
                        continue ;
                    delete it->second;
                    temporaryEntitiesRecords.erase(it->second);
                    entitiesDeleted++;
                }
                temporaryEntities.erase(name);
                SystemDestroyed sd;
                sd.name = name;
                events->send<SystemDestroyed>(sd);
                events->send<std::string>("[" + name + "] shutdown. Killed " + std::to_string(entitiesDeleted) + " entities.");
                counter -= entitiesDeleted;
                systemsMarkedForErase.pop();
                delete system;
                afterDeath(this);
                if (extensionFiles.find(name) != extensionFiles.end()) {
                    extensions.erase(extensionFiles[name]);
                    extensionFiles.erase(name);
                }
            }
        }

        int run()
        {
            try {
                auto elapsed = timeKeeper.loop();
                for (auto &pair: orderMap)
                {
                    auto &system = pair.second;
                    currentSystem = system;
                    system->run(elapsed);
                }
                cleanSystems();
            } catch (std::out_of_range const &)
            {
                if (!systemsMarkedForErase.empty()) {
                    std::cout << "Failed to erase " << systemsMarkedForErase.front() << std::endl;
                    systemsMarkedForErase.pop();
                }
//                throw ;
            }
            return 0;
        }

        ~EntityManager()
        {
            if (counter != 0)
                std::cerr << "Leaked memory : " << counter << " entities leaked." << std::endl;
            else
                std::cout << "Clean exit. Have a nice day !" << std::endl;
        }
    };
}

#endif //FEUCS_FEUCS_HPP
