#pragma once

#include <unordered_map>
#include <queue>
#include <vector>
#include <type_traits>
#include <functional>
#include <stdexcept>
#include <assert.h>

#ifdef DEBUG
#include <iostream>
#include <debugapi.h>
#include <string>
#endif // DEBUG


namespace rfe
{
	using ComponentTypeID = size_t;
	using ComponentIndex = size_t;
	using EntityID = size_t;

	class EntityComponentManager;
	class EntityReg;
	class ECSSerializer;


	class Entity
	{
		friend EntityComponentManager;
	public:
		Entity() : m_entityIndex(-1), m_entCompManRef(nullptr) {}
		~Entity();
		Entity(const Entity& other);
		Entity& operator=(const Entity& other);
		Entity(Entity&& other) noexcept;
		Entity& operator=(Entity&& other) noexcept;

		template<typename T>
		T* getComponent();

		//requires std::is_trivially_copy_assignable_v<T>
		template<typename T>
		requires std::is_copy_assignable_v<T>
			T* addComponent(const T& component) const;

		template<typename T>
		requires std::is_copy_assignable_v<T>
			T* addScript(const T& component) const;

		template<typename T>
		void removeComponent() const;

		void reset();
		EntityID getID() const { return m_entityIndex; }
		int getRefCount() const { return s_refCounts[m_entityIndex]; }
		bool empty() const
		{
#ifdef DEBUG
			if (m_entityIndex != -1)
			{
				assert(s_refCounts[m_entityIndex] > 0);
				assert(m_entCompManRef);
			}
			else
			{
				assert(!m_entCompManRef);
			}

#endif // DEBUG


			return m_entityIndex == -1 || !m_entCompManRef || s_refCounts[m_entityIndex] <= 0;
		}
		operator const EntityID() const { return m_entityIndex; }

	private:
		Entity(EntityID ID, EntityComponentManager* entReg);

		EntityID m_entityIndex;
		EntityComponentManager* m_entCompManRef;
		inline static std::vector<int> s_refCounts;
	};


	class BaseComponent
	{
		friend EntityComponentManager;
		friend ECSSerializer;

	public:
		EntityID getEntityID() const { return entityIndex; }
		Entity getEntity() const;
	protected:
		EntityID entityIndex = -1;

		static ComponentTypeID registerComponent(size_t size, std::string name, std::function<ComponentIndex(BaseComponent*)> createFunc,
			std::function<BaseComponent* (ComponentIndex)> fetchFunc, std::function<EntityID(ComponentIndex)> deleteFunc,
			std::function<void(size_t)> resize, std::function<char* ()> getArray, std::function<size_t()> count, std::function<void(ComponentIndex)> destroy);
	private:
		struct ComponentUtility
		{
			size_t size = 0;
			std::string name;
			std::function<ComponentIndex(BaseComponent*)> createComponent;
			std::function<BaseComponent* (ComponentIndex)> fetchComponentAsBase;
			std::function<EntityID(ComponentIndex)> deleteComponent;
			std::function<void(size_t)> resizeArrayT;
			std::function<char* ()> getArrayPointer;
			std::function<size_t()> componentCount;
			std::function<void(ComponentIndex)> compDestroy;
		};

		inline static std::vector<ComponentUtility> s_componentRegister;

		static size_t getSize(ComponentTypeID id);
		static std::function<ComponentIndex(BaseComponent*)> getCreateComponentFunction(ComponentTypeID id);
		static ComponentUtility getComponentUtility(ComponentTypeID id);
	};

	struct ComponentMetaData
	{
		ComponentMetaData(ComponentTypeID typeID, ComponentIndex index, EntityID entIndex, bool script) : typeID(typeID), index(index), entityIndex(entIndex), isScript(script) {}
		ComponentTypeID typeID;
		ComponentIndex index;
		EntityID entityIndex;
		bool isScript = false;
	};


	template<typename T>
	class Component : public BaseComponent
	{
		friend EntityComponentManager;
		friend EntityReg;

	private:
		using BaseComponent::entityIndex;
		using BaseComponent::registerComponent;

	public:
		static const ComponentTypeID typeID;
		static const size_t size;
		static const std::string componentName;

	private:
		static std::vector<T> componentArray;

		void destroy()
		{

		}

		template<typename T>
		static void destroy(ComponentIndex index)
		{
			componentArray[index].destroy();
		}

		//requires std::is_trivially_copy_assignable_v<T>
		template<typename T>
		requires std::is_copy_assignable_v<T>
			static ComponentIndex createComponent(BaseComponent* comp)
		{
			componentArray.push_back(*static_cast<T*>(comp));
			return componentArray.size() - 1;
		}
		template<typename T>
		static BaseComponent* fetchComponent(ComponentIndex index)
		{
			return static_cast<BaseComponent*>(&componentArray[index]);
		}
		template<typename T>
		static EntityID deleteComponent(ComponentIndex index)
		{
			EntityID entityOwningBackComponent = -1;
			if (index + 1 != componentArray.size()) // last component skip this step
			{
				componentArray[index] = componentArray.back();
				entityOwningBackComponent = componentArray[index].entityIndex;
			}
			componentArray.pop_back();
			return entityOwningBackComponent;
		}
		static void resizeComponentArray(size_t elements) { componentArray.resize(elements); }
		static char* getArrayStartPointer() { return reinterpret_cast<char*>(componentArray.data()); }
		static size_t getCount() { return componentArray.size(); }
	};

	template<typename T>
	std::vector<T> Component<T>::componentArray;

	template<typename T>
	const ComponentTypeID Component<T>::typeID = BaseComponent::registerComponent(sizeof(T), typeid(T).name(), Component<T>::createComponent<T>,
		Component<T>::fetchComponent<T>, Component<T>::deleteComponent<T>, Component<T>::resizeComponentArray, Component<T>::getArrayStartPointer,
		Component<T>::getCount, Component<T>::destroy<T>);

	template<typename T>
	const size_t Component<T>::size = sizeof(T);

	template<typename T>
	const std::string Component<T>::componentName = typeid(T).name();




	class EntityComponentManager
	{
		friend EntityReg;
		friend ECSSerializer;
		friend BaseComponent;
		EntityComponentManager() = default;
		EntityComponentManager(const EntityComponentManager&) = delete;
		EntityComponentManager& operator=(const EntityComponentManager&) = delete;
		~EntityComponentManager();

	public:
		Entity& createEntity();
		void removeEntity(Entity& entity); // this should be encapsulated better
		void addComponent(EntityID entityID, ComponentTypeID typeID, BaseComponent* component, bool script);

		//requires std::is_trivially_copy_assignable_v<T>
		template<typename T>
		requires std::is_copy_assignable_v<T>
			T* addComponent(EntityID entityID, const T& component);

		template<typename T>
		requires std::is_copy_assignable_v<T>
		T* AddScript(EntityID entityID, const T& comp);

		template<typename T>
		void removeComponent(EntityID entityID);

		template<typename T>
		T* getComponent(EntityID entityID);

	private:
		template<typename... T>
		void RunScripts(float dt);

		template<typename T>
		void RunScript(float dt);

		void removeComponent(ComponentTypeID type, EntityID entityID);
		void removeInternalEntity(Entity& entity);
		Entity createEntityFromExistingID(EntityID id) { return Entity(id, this); }

	private:
		std::vector<std::vector<ComponentMetaData>> m_entitiesComponentHandles;
		std::queue<EntityID> m_freeEntitySlots;
		std::vector<Entity> m_entityRegistry;
		bool clearHasBeenCalled = false;
	};


	class EntityReg
	{
		friend ECSSerializer;
		friend BaseComponent;
		EntityReg() = delete;
	public:
		static void clear();

		template<typename... Args>
		static void RunScripts(float dt);

		static Entity& createEntity();
		//static void removeEntity(Entity& entity);
		static void addComponent(EntityID entityID, ComponentTypeID typeID, BaseComponent* component, bool script);
		static const std::vector<Entity>& getAllEntities();

		template<typename T>
		requires std::is_copy_assignable_v<T>&& std::is_copy_constructible_v<T>
			static T* addComponent(EntityID entityID, const T& component);

		template<typename T>
		static void removeComponent(EntityID entityID);

		template<typename T>
		static T* getComponent(EntityID ID);

		template<typename T>
		static std::vector<T>& getComponentArray();
	private:
		inline static EntityComponentManager m_entCompManInstance;
	};

	//EntityReg definitions
	//----------------------------------------------------

	inline Entity& EntityReg::createEntity()
	{
		return m_entCompManInstance.createEntity();
	}

	inline void EntityReg::clear()
	{
		// this will delete alla entities and components, before componentesArray gets destroyed
		for (auto& e : m_entCompManInstance.m_entityRegistry)
		{
			if (!e.empty())
			{
				if (e.getRefCount() != 1)
				{
					OutputDebugString(L"[ERROR] Release all outstanding references to entities before calling EntityReg::clear(). \n\tEntityID: ");
					OutputDebugString(std::to_wstring(e.getID()).c_str());
					OutputDebugString(L"\n\tNumRefs needed to be released: ");
					OutputDebugString(std::to_wstring(e.getRefCount() - 1).c_str());
					OutputDebugString(L"\n");
					throw std::runtime_error("release all outstanding references to entities before calling EntityReg::clear().");
				}
				m_entCompManInstance.removeEntity(e);
			}
		}
		m_entCompManInstance.m_entityRegistry.clear();
		m_entCompManInstance.clearHasBeenCalled = true;
	}


	template<typename ...Args>
	inline void EntityReg::RunScripts(float dt)
	{
		//int e[]{ 0, (m_entCompManInstance.RunScripts<Args>(dt), 0)... };
		m_entCompManInstance.RunScripts<Args...>(dt);
	}


	/*inline void EntityReg::removeEntity(Entity& entity)
	{
		assert(&entity != &m_entCompManInstance.m_entityRegistry[entity.getID()]);
		m_entCompManInstance.removeEntity(entity);
	}*/

	inline void EntityReg::addComponent(EntityID entityID, ComponentTypeID typeID, BaseComponent* component, bool script)
	{
		m_entCompManInstance.addComponent(entityID, typeID, component, script);
	}

	
	template<typename T>
	requires std::is_copy_assignable_v<T>&& std::is_copy_constructible_v<T>
		inline T* EntityReg::addComponent(EntityID entityID, const T& component)
	{
		return m_entCompManInstance.addComponent<T>(entityID, component);
	}

	template<typename T>
	inline void EntityReg::removeComponent(EntityID entityID)
	{
		m_entCompManInstance.removeComponent<T>(entityID);
	}

	template<typename T>
	inline T* EntityReg::getComponent(EntityID ID)
	{
		return m_entCompManInstance.getComponent<T>(ID);
	}

	template<typename T>
	inline std::vector<T>& EntityReg::getComponentArray()
	{
		return T::componentArray;
	}

	inline const std::vector<Entity>& EntityReg::getAllEntities()
	{
		return m_entCompManInstance.m_entityRegistry;
	}



	//Entity definitions
	//-------------------------------------
	inline Entity::Entity(EntityID ID, EntityComponentManager* entReg) : m_entityIndex(ID), m_entCompManRef(entReg)
	{
		if (ID >= s_refCounts.size())
		{
			s_refCounts.push_back(1);
		}
		else
		{
			s_refCounts[ID]++;
		}
	}

	inline Entity::Entity(const Entity& other)
	{
		if (!other.empty())
		{
			this->m_entCompManRef = other.m_entCompManRef;
			this->m_entityIndex = other.m_entityIndex;
			s_refCounts[m_entityIndex]++;
		}
		else
		{
			this->m_entCompManRef = nullptr;
			this->m_entityIndex = -1;
		}
	}

	inline Entity::~Entity()
	{

#ifdef _DEBUG
		if (!this->empty())
		{
			OutputDebugString(L"~Entity\tindex: ");
			OutputDebugString(std::to_wstring(m_entityIndex).c_str());
			OutputDebugString(L", refCount: ");
			OutputDebugString(std::to_wstring(s_refCounts[m_entityIndex]).c_str());
			OutputDebugString(L"\n");
		}
#endif // _DEBUG
		this->reset();
	}

	inline Entity& Entity::operator=(const Entity& other)
	{
		this->reset();
		if (!other.empty())
		{
			this->m_entCompManRef = other.m_entCompManRef;
			this->m_entityIndex = other.m_entityIndex;
			s_refCounts[m_entityIndex]++;
		}
		else
		{
			this->m_entCompManRef = nullptr;
			this->m_entityIndex = -1;
		}
		return *this;
	}

	inline Entity::Entity(Entity&& other) noexcept
	{
		this->m_entityIndex = other.m_entityIndex;
		this->m_entCompManRef = other.m_entCompManRef;
		//invalidate other
		other.m_entCompManRef = nullptr;
		other.m_entityIndex = -1;
	}

	inline Entity& Entity::operator=(Entity&& other) noexcept
	{
		this->reset();
		this->m_entityIndex = other.m_entityIndex;
		this->m_entCompManRef = other.m_entCompManRef;
		//invalidate other
		other.m_entCompManRef = nullptr;
		other.m_entityIndex = -1;
		return *this;
	}
	inline void Entity::reset()
	{
		if (empty()) return;
		if (s_refCounts[m_entityIndex] <= 2)
		{
			m_entCompManRef->removeEntity(*this);
			assert(this->empty()); //removeEntity take care of reseting the entity
		}
		else
		{
			s_refCounts[m_entityIndex]--;
			m_entityIndex = -1;
			m_entCompManRef = nullptr;
		}
	};

	template<typename T>
	inline T* Entity::getComponent()
	{
		return m_entCompManRef->getComponent<T>(this->m_entityIndex);
	}

	//requires std::is_trivially_copy_assignable_v<T>
	template<typename T>
	requires std::is_copy_assignable_v<T>
		inline T* Entity::addComponent(const T& component) const
	{
		return m_entCompManRef->addComponent<T>(this->m_entityIndex, component);
	}

	template<typename T>
	requires std::is_copy_assignable_v<T>
	inline T* Entity::addScript(const T& component) const
	{
		return m_entCompManRef->AddScript<T>(this->m_entityIndex, component);
	}

	template<typename T>
	inline void Entity::removeComponent() const
	{
		m_entCompManRef->removeComponent<T>(this->m_entityIndex);
	}


	//EntityComponentManager definitions
	//----------------------------------------------------

	inline EntityComponentManager::~EntityComponentManager()
	{
		if (!clearHasBeenCalled)
		{
			OutputDebugString(L"[ERROR] call clear on EntityReg before main returns.\n");
			OutputDebugString(L"[ERROR] call clear on EntityReg before main returns.\n");
			OutputDebugString(L"[ERROR] call clear on EntityReg before main returns.\n");
			OutputDebugString(L"[ERROR] call clear on EntityReg before main returns.\n");
			OutputDebugString(L"[ERROR] call clear on EntityReg before main returns.\n");
			OutputDebugString(L"[ERROR] call clear on EntityReg before main returns.\n");
			assert(false);
		}
	}

	inline Entity& EntityComponentManager::createEntity()
	{
		EntityID index;
		if (!m_freeEntitySlots.empty())
		{
			index = m_freeEntitySlots.front();
			m_freeEntitySlots.pop();
			m_entitiesComponentHandles[index].clear();
			m_entityRegistry[index] = std::move(Entity(index, this));
		}
		else
		{
			index = m_entitiesComponentHandles.size();
			m_entitiesComponentHandles.emplace_back(std::vector<ComponentMetaData>());
			m_entityRegistry.emplace_back(Entity(index, this));
			assert(m_entityRegistry.back().getID() == m_entityRegistry[index].getID());
		}
		return m_entityRegistry[index];
	}

	inline void EntityComponentManager::removeInternalEntity(Entity& entity)
	{
		assert(!entity.empty());
		assert(entity.s_refCounts[entity.m_entityIndex] == 1);
		assert(m_entitiesComponentHandles.size() == m_entityRegistry.size());
		assert(&entity == &m_entityRegistry[entity.m_entityIndex]);





		auto& components = m_entitiesComponentHandles[entity.m_entityIndex];
		for (auto& c : components)
		{
			auto compUtil = BaseComponent::getComponentUtility(c.typeID);
			compUtil.compDestroy(c.index);

			removeComponent(c.typeID, entity.m_entityIndex);
		}
		if (entity.m_entityIndex + 1 == m_entitiesComponentHandles.size())
		{
			m_entitiesComponentHandles.pop_back();
		}
		else
		{
			m_entitiesComponentHandles[entity.m_entityIndex].clear();
			m_freeEntitySlots.push(entity.m_entityIndex);
		}

		//reset
		entity.s_refCounts[entity.m_entityIndex] = 0;
		entity.m_entCompManRef = nullptr;
		entity.m_entityIndex = -1;
	}

	inline void EntityComponentManager::removeEntity(Entity& entity)
	{
		assert(!entity.empty());

#ifdef DEBUG
		std::string debugOut = "Removed Entity: " + std::to_string(entity.m_entityIndex) + ", refCount: "
			+ std::to_string(entity.s_refCounts[entity.m_entityIndex]) + "\n";
		std::cout << debugOut;
#endif // DEBUG

		if (entity.s_refCounts[entity.m_entityIndex] > 2)
		{

			std::string debugOut = "[ERROR] release " + std::to_string(entity.s_refCounts[entity.m_entityIndex] - 2) +
				" references to Entity, ID: " + std::to_string(entity.m_entityIndex) +
				"\n\tOnly two references is allowd when removeEntity(Entity) is called.\n\tOne internal and one for the argument to the call.\n";
#ifdef DEBUG
			std::cout << debugOut;
#endif // DEBUG



			throw std::runtime_error(debugOut);
		}


		if (entity.getRefCount() == 1)
		{
			removeInternalEntity(entity);
		}
		else if (entity.getRefCount() == 2)
		{
			EntityID id = entity.getID();

			//first reset entity: ref count will now be 1, can't call entity.reset(), because that will lead to recursion
			entity.m_entCompManRef = nullptr;
			entity.s_refCounts[entity.m_entityIndex]--;
			entity.m_entityIndex = -1;

			if (id + 1 == m_entitiesComponentHandles.size())
			{
				m_entityRegistry.pop_back(); //second reset internal entity
			}
			else
			{
				m_entityRegistry[id].reset(); //second reset internal entity
			}
			assert(entity.s_refCounts[id] == 0);
		}





		//return;
		////----------------------------


		//

		//auto& components = m_entitiesComponentHandles[entity.m_entityIndex];
		//for (auto& c : components)
		//{
		//	removeComponent(c.typeID, entity.m_entityIndex);
		//}
		////this is the entity in the back of the vector
		//if (entity.m_entityIndex + 1 == m_entitiesComponentHandles.size())
		//{
		//	m_entityRegistry.pop_back();
		//	m_entitiesComponentHandles.pop_back();
		//}
		//else
		//{
		//	m_entitiesComponentHandles[entity.m_entityIndex].clear();
		//	m_freeEntitySlots.push(entity.m_entityIndex);
		//}

	}

	inline void EntityComponentManager::addComponent(EntityID entityID, ComponentTypeID typeID, BaseComponent* component, bool script)
	{
		auto createFunc = BaseComponent::getCreateComponentFunction(typeID);
		ComponentIndex index = createFunc(component);
		m_entitiesComponentHandles[entityID].emplace_back(typeID, index, entityID, script);
	}


	//requires std::is_trivially_copy_assignable_v<T>
	template<typename T>
	requires std::is_copy_assignable_v<T>
		inline T* EntityComponentManager::addComponent(EntityID entityID, const T& comp)
	{
		ComponentTypeID typeID = T::typeID;
		ComponentIndex index;
		index = comp.componentArray.size();
		T::componentArray.emplace_back(comp);

		T* compPtr = &T::componentArray[index];
		compPtr->entityIndex = entityID;
		m_entitiesComponentHandles[entityID].emplace_back(typeID, index, entityID, false);
		return compPtr;
	}

	template<typename T>
	requires std::is_copy_assignable_v<T>
		inline T* EntityComponentManager::AddScript(EntityID entityID, const T& comp)
	{
		ComponentTypeID typeID = T::typeID;
		ComponentIndex index;
		index = comp.componentArray.size();
		T::componentArray.emplace_back(comp);

		T* compPtr = &T::componentArray[index];
		compPtr->entityIndex = entityID;
		m_entitiesComponentHandles[entityID].emplace_back(typeID, index, entityID, true);
		return compPtr;
	}

	template<typename T>
	inline void EntityComponentManager::removeComponent(EntityID entityID)
	{
		removeComponent(T::typeID, entityID);
	}

	template<typename T>
	class NativeScriptComponent : public Component<T>
	{
		/*friend EntityComponentManager;
		friend Component<T>;
	private:
		using BaseComponent::entityIndex;
		using BaseComponent::registerComponent;*/

	public:
		//helpers functions
		template<typename T>
		T* getComponent()
		{
			return EntityReg::getComponent<T>(this->getEntityID());
		}

		//On update functions
		void OnUpdate(float dt) {};
	};

	template<typename... T>
	inline void EntityComponentManager::RunScripts(float dt)
	{
		int e[]{ 0, (RunScript<T>(dt), 0)... };
	}

	template<typename T>
	inline void EntityComponentManager::RunScript(float dt)
	{
		for (auto& script : T::componentArray)
		{
			script.OnUpdate(dt);
		}
	}

	inline void EntityComponentManager::removeComponent(ComponentTypeID type, EntityID entityID)
	{
		auto& entityComponentHandles = m_entitiesComponentHandles[entityID];
		if (auto iti = std::ranges::find_if(entityComponentHandles.begin(), entityComponentHandles.end(),
			[type](ComponentMetaData c) { return type == c.typeID; });
			iti != entityComponentHandles.end())
		{
			auto componentUtilityFunctions = BaseComponent::getComponentUtility(iti->typeID);
			if (EntityID movedCompEntity = componentUtilityFunctions.deleteComponent(iti->index); movedCompEntity != -1)
			{
				auto& movedCompEntityHandles = m_entitiesComponentHandles[movedCompEntity];
				if (auto itj = std::ranges::find_if(movedCompEntityHandles.begin(), movedCompEntityHandles.end(),
					[type](ComponentMetaData c) { return type == c.typeID; });
					itj != movedCompEntityHandles.end())
				{
					itj->index = iti->index;
				}
				else
				{
					assert(false); // find_if above should not fail
				}
			}
			*iti = entityComponentHandles.back();
			entityComponentHandles.pop_back();
		}
	}

	template<typename T>
	inline T* EntityComponentManager::getComponent(EntityID entityID)
	{
		auto& entityComponents = m_entitiesComponentHandles[entityID];
		if (auto it = std::ranges::find_if(entityComponents.begin(), entityComponents.end(),
			[](ComponentMetaData c) { return T::typeID == c.typeID; });
			it != entityComponents.end())
		{
			auto r = &T::componentArray[it->index];
			return r;
		}
		return nullptr;
	}


	//BaseCompoent definitions
	//---------------------------------------------------
	inline ComponentTypeID BaseComponent::registerComponent(size_t size, std::string name, std::function<ComponentIndex(BaseComponent*)> createFunc,
		std::function<BaseComponent* (ComponentIndex)> fetchFunc, std::function<EntityID(ComponentIndex)> deleteFunc,
		std::function<void(size_t)> resize, std::function<char* ()> getArray, std::function<size_t()> count, std::function<void(ComponentIndex)> destroy)
	{
		ComponentTypeID compID = s_componentRegister.size();
		ComponentUtility comUtil;
		comUtil.size = size;
		comUtil.name = std::move(name);
		comUtil.createComponent = createFunc;
		comUtil.fetchComponentAsBase = fetchFunc;
		comUtil.deleteComponent = deleteFunc;
		comUtil.getArrayPointer = getArray;
		comUtil.resizeArrayT = resize;
		comUtil.componentCount = count;
		comUtil.compDestroy = destroy;
		s_componentRegister.push_back(comUtil);
		return compID;
	}

	inline Entity BaseComponent::getEntity() const
	{
		return EntityReg::m_entCompManInstance.createEntityFromExistingID(entityIndex);
	}

	inline size_t BaseComponent::getSize(ComponentTypeID id)
	{
		return s_componentRegister[id].size;
	}
	inline std::function<ComponentIndex(BaseComponent*)> BaseComponent::getCreateComponentFunction(ComponentTypeID id)
	{
		return s_componentRegister[id].createComponent;
	}
	inline BaseComponent::ComponentUtility BaseComponent::getComponentUtility(ComponentTypeID id)
	{
		return s_componentRegister[id];
	}
}
