#pragma once

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <meltdown/math.hpp>

namespace mtd
{
	/*
	* @brief Base class for any engine object.
	*/
	class Model
	{
		public:
			/*
			* @brief Creates a model instance, setting its pre transform matrix.
			*
			* @param modelID String uniquely identifying the model type.
			* @param preTransform Matrix with the starting transformation of the model instance.
			*/
			Model(const char* modelID, const Mat4x4& preTransform);
			Model() = delete;
			virtual ~Model();

			Model(const Model&) = delete;
			Model& operator=(const Model&) = delete;

			/*
			* @brief Gets a constant reference to the 4x4 transformation matrix of the model.
			*
			* @return Constant reference to the model transfomation matrix.
			*/
			const Mat4x4& getTransform() const;
			/*
			* @brief Gets a reference to the 4x4 transformation matrix of the model.
			*
			* @return Reference to the model transfomation matrix.
			*/
			Mat4x4& getTransform();
			/*
			* @brief Gets the instance ID for this model instance.
			*
			* @return Numeric ID uniquely identifying every instance in the scene.
			*/
			uint64_t getInstanceID() const;

			/*
			* @brief Runs once at the beginning of the scene.
			*/
			virtual void start() = 0;
			/*
			* @brief Runs on every frame update.
			*
			* @param deltaTime Time, in seconds, between the last frame and the current one.
			*/
			virtual void update(double deltaTime) = 0;

		protected:
			/*
			* @brief Transformation matrix for the mesh related to the model.
			*/
			Mat4x4 transform;

		private:
			/*
			* @brief Unique ID for every instance in the current scene.
			*/
			uint64_t instanceID;
	};

	using ModelFactory = std::function<std::unique_ptr<Model>(const Mat4x4&)>;
	using ModelFactories = std::unordered_map<std::string, mtd::ModelFactory>;

	/*
	* @brief Stores and handles model instantiation, which describes an object in the engine.
	*/
	class ModelHandler
	{
		public:
			ModelHandler() = delete;
			ModelHandler(const ModelHandler&) = delete;
			ModelHandler& operator=(const ModelHandler&) = delete;

			/*
			* @brief Returns a model factory for a model, if registered.
			*
			* @param modelID String identifying a specific registered model.
			*
			* @return A function used for instancing a model.
			*/
			static ModelFactory getModelFactory(const std::string& modelID);

			/*
			* @brief Returns a pointer to the model instance associated to an ID.
			*
			* @param instanceID Unique number identifying all instances present in the current scene.
			*
			* @return Pointer to the requested model instance, or a `nullptr` if there is no corresponding instance.
			*/
			static Model* getModelInstanceByID(uint64_t instanceID);

			/*
			* @brief Returns the unordered map containing all model instances in the scene.
			*
			* @return Reference to the model instances unordered map.
			*/
			static const std::unordered_map<uint64_t, Model*>& getModelInstancesMap();

			/*
			* @brief Stores a new model, which will be linked based on the model ID.
			*
			* @param modelID String identifying the model to be registered.
			*/
			template<typename ModelT>
			static void registerModel(const char* modelID)
			{
				static_assert
				(
					std::is_base_of<Model, ModelT>::value,
					"ModelT must be a derived class of Model in registerModel<ModelT>()."
				);

				modelFactoryRegistry[modelID] = [&modelID](const Mat4x4& preTransform)
				{
					return std::make_unique<ModelT>(modelID, preTransform);
				};
			}

		private:
			/*
			* @brief Stores the callbacks to create new model instances.
			*/
			static ModelFactories modelFactoryRegistry;
	};
}
