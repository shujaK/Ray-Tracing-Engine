#include <fstream>

#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"
#include "Camera.h"

#include "glm/gtc/type_ptr.hpp"

#include "inc/json.hpp"

using namespace Walnut;

class MainLayer : public Walnut::Layer
{
public:
	MainLayer() : m_Camera(45.0f, 0.1f, 100.0f)
	{

		Material& pinkSphere = m_Scene.Materials.emplace_back();
		pinkSphere.Albedo = { 1.0f, 0.0f, 1.0f };
		pinkSphere.Roughness = 0.15f;
		Material& blueSphere = m_Scene.Materials.emplace_back();
		blueSphere.Albedo = { 0.0f, 0.0f, 1.0f };
		blueSphere.Roughness = 0.1f;
		Material& lightSphere = m_Scene.Materials.emplace_back();
		lightSphere.Albedo = { 0.8f, 0.5f, 0.2f };
		lightSphere.Roughness = 0.1f;
		lightSphere.EmissionColor = lightSphere.Albedo;
		lightSphere.EmissionStrength = 2;


        pSphere sphere1;
        sphere1.Position = { 0.0f, 0.0f, 0.0f };
        sphere1.Radius = 1.0f;
		sphere1.MaterialIndex = 0;
        m_Scene.ProcObjects.add(std::make_shared<pSphere> (sphere1));

        pSphere sphere2;
        sphere2.Position = { 0.0f, -101.0f, 0.0f };
        sphere2.Radius = 100.0f;
		sphere2.MaterialIndex = 1;
		m_Scene.ProcObjects.add(std::make_shared<pSphere>(sphere2));

		pSphere sphere3;
		sphere3.Position = { 2.0f, -0.0f, 0.0f };
		sphere3.Radius = 1;
		sphere3.MaterialIndex = 2;
		m_Scene.ProcObjects.add(std::make_shared<pSphere>(sphere3));
	}

	virtual void OnUpdate(float ts) override
	{
		if (m_Camera.OnUpdate(ts))
		{
			m_Renderer.ResetFrameIndex();
		}
	}


    virtual void OnUIRender() override
    {
		bool UIEdited = false;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;


		if (imageMousePos.x > -1 && ImGui::IsMouseDown(0))
		{
			auto p = m_Renderer.ClickQueryObject(imageMousePos.x, m_ViewportHeight - imageMousePos.y);
			selectedObjectIndex = p.ObjectIndex;
		}

		auto finalImage = m_Renderer.GetFinalImage();
		if (finalImage)
		{
			ImVec2 imageSize = { (float) finalImage->GetWidth(), (float) finalImage->GetHeight() };
			ImGui::Image(finalImage->GetDescriptorSet(), imageSize, ImVec2(0, 1), ImVec2(1, 0));

			// Get the mouse position relative to the image
			ImVec2 mousePos = ImGui::GetMousePos();
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 relativeMousePos = { mousePos.x - windowPos.x, mousePos.y - windowPos.y };

			// Check if the mouse is outside the bounds of the image
			if (relativeMousePos.x < 0 || relativeMousePos.x > imageSize.x || relativeMousePos.y < 0 || relativeMousePos.y > imageSize.y)
			{
				relativeMousePos.x = -1;
				relativeMousePos.y = -1;
			}

			imageMousePos.x = relativeMousePos.x;
			imageMousePos.y = relativeMousePos.y;
		}

		ImGui::End();
		ImGui::PopStyleVar();

        ImGui::Begin("Settings");
        ImGui::Text("Last render: %.3fms (%.2f FPS)", m_LastRenderTime, 1000.0f/ m_LastRenderTime);
		UIEdited |= ImGui::Checkbox("dbgToggle", &m_Renderer.GetSettings().debugToggle);
		// ImGui::Text("Frame #: %d", m_Renderer.GetFrameIndex());

		//plot fps in an ImGui::PlotLines
		static float values[90] = { 0 };
		static int values_offset = 0;
		values[values_offset] = 1000.0f / m_LastRenderTime;
		values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
		ImGui::PlotLines("FPS", values, IM_ARRAYSIZE(values), values_offset, nullptr, 0.0f, 200.0f, ImVec2(0, 80.0f));

		if (!realtime)
		{
			if (ImGui::Button("Render"))
			{
				Render();
			}
		}
		ImGui::Checkbox("Realtime Render", &realtime);
		if (ImGui::TreeNode("Anti Aliasing"))
		{
			UIEdited |= ImGui::Checkbox("Enabled", &m_Renderer.GetSettings().Antialiasing);
			UIEdited |= ImGui::InputInt("Samples", &m_Renderer.GetSettings().AntialiasingSamples);
			UIEdited |= ImGui::InputInt("Factor", &m_Renderer.GetSettings().AntialiasingFactor);
			ImGui::TreePop();
		}
		UIEdited |= ImGui::InputInt("Light Bounces", &m_Renderer.GetSettings().Bounces);
		ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);
		UIEdited |= ImGui::DragFloat("Clip Distance", &m_Renderer.GetSettings().FarDistance, 0.01f, 0.0f, 10000000000000.0f);
		if (ImGui::Button("Reset"))
		{
			m_Renderer.ResetFrameIndex();
		}

        ImGui::End();

		ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_MenuBar);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Add"))
			{
				if (ImGui::MenuItem("Sphere"))
				{
					m_Scene.addSphere();
					selectedObjectIndex = m_Scene.ProcObjects.Objects.size() - 1;
					UIEdited = true;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Save/Load"))
			{
				if (ImGui::MenuItem("Save Scene"))
				{
					// SaveScene("scene");

				}
				if (ImGui::MenuItem("Load Scene"))
				{
					// LoadScene();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::TreeNode("Object"))
		{
            if (selectedObjectIndex > -1)
            {
				auto object = m_Scene.ProcObjects.Objects[selectedObjectIndex].get();
				object->DisplayMenu(&UIEdited);
            	if (object->getMaterialIndex() < 0) {object->setMaterialIndex(0); UIEdited = false;}
				if (object->getMaterialIndex() >= m_Scene.Materials.size()) {object->setMaterialIndex(m_Scene.Materials.size() - 1); UIEdited = false;}
				
				if (ImGui::Button("Delete"))
				{
					m_Scene.ProcObjects.Objects.erase(m_Scene.ProcObjects.Objects.begin() + selectedObjectIndex);
					selectedObjectIndex = -1;
					UIEdited = true;
				}
				ImGui::Separator();
				if (selectedObjectIndex != -1 ) 
				{
					auto& mat = m_Scene.Materials[object->getMaterialIndex()];
					UIEdited |= ImGui::InputInt("Material Type", &mat.type);
					UIEdited |= ImGui::ColorEdit3("Albedo", glm::value_ptr(mat.Albedo), 0.01f);
					UIEdited |= ImGui::DragFloat("Roughness", &mat.Roughness, 0.01f, 0.0f, 1.0f);
					UIEdited |= ImGui::DragFloat("Metallic", &mat.Metallic, 0.01f, 0.0f, 1.0f);
					UIEdited |= ImGui::ColorEdit3("Emission Color", glm::value_ptr(mat.EmissionColor), 0.01f);
					UIEdited |= ImGui::DragFloat("Emission Strength", &mat.EmissionStrength, 0.1f, 0.0f, 5000.0f);
					ImGui::Separator();
				}
            }
            ImGui::TreePop();
            }

        if (ImGui::TreeNode("Materials"))
        {
			for (size_t i = 0; i < m_Scene.Materials.size(); i++)
			{
				if (ImGui::TreeNode(("Material " + std::to_string(i)).c_str()))
				{
					ImGui::PushID(i);
					UIEdited |= ImGui::InputInt("Material Type", &m_Scene.Materials[i].type);
					UIEdited |= ImGui::ColorEdit3("Albedo", glm::value_ptr(m_Scene.Materials[i].Albedo), 0.01f);
					UIEdited |= ImGui::DragFloat("Roughness", &m_Scene.Materials[i].Roughness, 0.01f, 0.0f, 1.0f);
					UIEdited |= ImGui::DragFloat("Metallic", &m_Scene.Materials[i].Metallic, 0.01f, 0.0f, 1.0f);
					UIEdited |= ImGui::ColorEdit3("Emission Color", glm::value_ptr(m_Scene.Materials[i].EmissionColor), 0.01f);
					UIEdited |= ImGui::DragFloat("Emission Strength", &m_Scene.Materials[i].EmissionStrength, 0.2f, 0.0f, 5000.0f);
					ImGui::Separator();
					ImGui::PopID();
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
        }

		if (ImGui::TreeNode("World"))
		{
			UIEdited |= ImGui::ColorEdit3("Sky Colour", glm::value_ptr(m_Scene.SkyColour), 0.01f);
			ImGui::TreePop();
		}

		ImGui::End();

		if (UIEdited) m_Renderer.ResetFrameIndex();
		if (realtime) Render();
    }

	void Render()
	{
		Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);

		m_Renderer.Render(m_Camera, m_Scene);

		m_LastRenderTime = timer.ElapsedMillis();
	}

	// void SaveScene(std::string filename)
	// {
	// 	std::ofstream file(filename + ".json");
	// 	if (file.is_open())
	// 	{
	// 		nlohmann::json j;
	// 		j["Spheres"] = m_Scene.Spheres;
	// 		j["Materials"] = m_Scene.Materials;
	// 		file << j.dump(4);
	// 		file.close();
	// 	}
	// }
	// 
	// void LoadScene()
	// {
	// 	std::ifstream file("scene.json");
	// 	if (file.is_open())
	// 	{
	// 		nlohmann::json j;
	// 		file >> j;
	// 		m_Scene.Spheres = j["Spheres"].get<std::vector<Sphere>>();
	// 		m_Scene.Materials = j["Materials"].get<std::vector<Material>>();
	// 		file.close();
	// 	}
	// 
	// 	m_Renderer.ResetFrameIndex();
	// }
private:
	bool realtime = true;
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	float m_LastRenderTime = 0.0f;
	glm::vec2 imageMousePos = { -1, -1 };
	int selectedObjectIndex = 0;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<MainLayer>();
	auto layer = app->PeekLayer();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}