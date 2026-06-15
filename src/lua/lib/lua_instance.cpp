#include "lua_instance.h"
#include "entt/entity/entity.hpp"
#include "entt/entity/fwd.hpp"
#include "util/assimp.h"
#include "util/output.h"
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

static Output output;

namespace Lua::instance {

	Ecs::InstanceComponent &get_instance(entt::registry *registry, entt::entity entity) {
		if (!registry->any_of<Ecs::InstanceComponent>(entity)) {
			throw std::runtime_error("ECS entity has no Instance?");
		}

		return registry->get<Ecs::InstanceComponent>(entity);
	}

	/*
	 * @namespace Instance
	 * This is an autodoc tool test
	 */
	void init(EngineInstance *engine, sol::state &lua) {
		registry = &engine->registry;

		lua.new_usertype<Instance>("Instance",
		                           /*
		                            * @field new
		                            * @type function
		                            * @param name string
		                            * Creates a new blank instance with the provided name.
		                            */
		                           sol::constructors<Instance(std::string)>(),

		                           /*
		                            * @field parent
		                            * @type field
		                            * Parent of entity.
		                            */
		                           "parent",
		                           sol::property(
		                               [](Instance &i) -> std::optional<Instance> {
			                               entt::entity parent = get_instance(registry, i.entity).parent;
			                               if (parent == entt::null) return std::nullopt;

			                               return Instance(parent);
		                               },
		                               [](Instance &i, Instance parent) {
			                               get_instance(registry, i.entity).parent = parent.entity;
		                               }),

		                           /*
		                            * @field name
		                            * @type field
		                            * Name of entity.
		                            */
		                           "name",
		                           sol::property(
		                               [](Instance &i) -> std::string {
			                               return get_instance(registry, i.entity).name;
		                               },
		                               [](Instance &i, std::string name) {
			                               get_instance(registry, i.entity).name = name;
		                               }),

		                           /*
		                            * @field children
		                            * @type field Instance[]
		                            * Instances parented to this one.
		                            */
		                           "children",
		                           sol::property([](Instance &i) -> std::vector<Instance> {
			                           std::vector<Instance> children;

			                           for (entt::entity entity : registry->view<Ecs::InstanceComponent>()) {
				                           if (registry->get<Ecs::InstanceComponent>(entity).parent == i.entity) {
					                           children.emplace_back(entity);
				                           }
			                           }

			                           return children;
		                           }),

		                           /*
		                            * @field transform
		                            * @type field Mat4
		                            * Local transform of the entity.
		                            */
		                           "transform",
		                           sol::property(
		                               [](Instance &i) -> glm::mat4 {
			                               return get_instance(registry, i.entity).transform;
		                               },
		                               [](Instance &i, glm::mat4 transform) {
			                               get_instance(registry, i.entity).transform = transform;
		                               })

		);
	}

	Lua::instance::Instance::Instance(std::string name) {
		entt::entity entity = registry->create();
		registry->emplace<Ecs::InstanceComponent>(entity).name = name;
		this->entity = entity;
	};

	Lua::instance::Instance::Instance(aiNode *node) {
		entt::entity entity = registry->create();
		registry->emplace<Ecs::InstanceComponent>(entity).name = node->mName.data;
		this->entity = entity;
	}

	Lua::instance::Instance::Instance(entt::entity entity) {
		this->entity = entity;
	};

	void Lua::instance::Instance::set_parent(Instance parent) {
		auto &instance = registry->get<Ecs::InstanceComponent>(entity);
		instance.parent = parent.entity;
	}

	void Lua::instance::Instance::set_transform(const aiMatrix4x4 &transform) {
		auto &instance = registry->get<Ecs::InstanceComponent>(entity);
		instance.transform = AssimpGLMHelpers::get_glm_matrix(transform);
	}
}
