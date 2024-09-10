#pragma once

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>

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
			*/
			Model(const Mat4x4& preTransform) : transform{preTransform} {}

			Model() = delete;
			virtual ~Model() = default;

			/*
			* @brief Gets the pointer to the 4x4 transformation matrix of the model.
			*/
			Mat4x4* getTransformPointer() { return &transform; }

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

				modelFactoryRegistry[modelID] = [](const Mat4x4& preTransform)
				{
					return std::make_unique<ModelT>(preTransform);
				};
			}

			/*
			* @brief Returns a model factory for a model, if registered.
			*
			* @param modelID String identifying a specific registered model.
			*
			* @return A function used for instancing a model.
			*/
			static ModelFactory getModelFactory(const std::string& modelID);

		private:
			/*
			* @brief Stores the callbacks to create new model instances.
			*/
			static ModelFactories modelFactoryRegistry;
	};
}
