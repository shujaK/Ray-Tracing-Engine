#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"
#include "Camera.h"

#include "glm/gtc/type_ptr.hpp"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer() : m_Camera(45.0f, 0.1f, 100.0f)
	{

		Material& pinkSphere = m_Scene.Materials.emplace_back();
		pinkSphere.Albedo = { 1.0f, 0.0f, 1.0f };
		pinkSphere.Roughness = 0.0f;
		Material& blueSphere = m_Scene.Materials.emplace_back();
		blueSphere.Albedo = { 0.0f, 0.0f, 1.0f };
		blueSphere.Roughness = 0.1f;


        Sphere sphere1;
        sphere1.Position = { 0.0f, 0.0f, 0.0f };
        sphere1.Radius = 1.0f;
		sphere1.MaterialIndex = 0;
        m_Scene.Spheres.push_back(sphere1);

        Sphere sphere2;
        sphere2.Position = { 0.0f, -101.0f, 0.0f };
        sphere2.Radius = 100.0f;
		sphere2.MaterialIndex = 1;
        m_Scene.Spheres.push_back(sphere2);
	}

	virtual void OnUpdate(float ts) override
	{
		m_Camera.OnUpdate(ts);
	}

    virtual void OnUIRender() override
    {
        ImGui::Begin("Settings");
        ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (!realtime)
		{
			if (ImGui::Button("Render"))
			{
				Render();
			}
		}
		ImGui::Checkbox("Realtime Render", &realtime);
        ImGui::End();

		ImGui::Begin("Scene");

		if (ImGui::TreeNode("Object"))
		{
			if (selectedObjectIndex > -1)
			{
				auto& sphere = m_Scene.Spheres[selectedObjectIndex];
				ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.01f);
				ImGui::DragFloat("Radius", &sphere.Radius, 0.01f, 0.0f);
				ImGui::DragInt("Material Index", &sphere.MaterialIndex, 1, 0.0f, m_Scene.Materials.size() - 1);
				ImGui::Separator();
				auto& mat = m_Scene.Materials[sphere.MaterialIndex];
				ImGui::ColorEdit3("Albedo", glm::value_ptr(mat.Albedo), 0.01f);
				ImGui::DragFloat("Roughness", &mat.Roughness, 0.01f, 0, 1);
				ImGui::DragFloat("Metallic", &mat.Metallic, 0.01f, 0, 1);
				ImGui::Separator();
			}
			ImGui::TreePop();
		}

		// for (size_t i = 0; i < m_Scene.Spheres.size(); i++)
		// {
		// 	ImGui::PushID(i);
		// 	auto& sphere = m_Scene.Spheres[i];
		// 	ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.01f);
		// 	ImGui::DragFloat("Radius", &sphere.Radius, 0.01f, 0.0f);
		// 	ImGui::DragInt("Material Index", &sphere.MaterialIndex, 1, 0.0f, m_Scene.Materials.size() - 1);
		// 	ImGui::Separator();
		// 	ImGui::PopID();
		// }

		if (ImGui::TreeNode("Materials"))
		{
			for (size_t i = 0; i < m_Scene.Materials.size(); i++)
			{
				if (ImGui::TreeNode(("Material " + std::to_string(i)).c_str()))
				{
					ImGui::PushID(i);
					ImGui::ColorEdit3("Albedo", glm::value_ptr(m_Scene.Materials[i].Albedo), 0.01f);
					ImGui::DragFloat("Roughness", &m_Scene.Materials[i].Roughness, 0.01f, 0, 1);
					ImGui::DragFloat("Metallic", &m_Scene.Materials[i].Metallic, 0.01f, 0, 1);
					ImGui::Separator();
					ImGui::PopID();
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}

		// ImGui::Text("Mouse Position: (%.1f, %.1f)", imageMousePos.x, imageMousePos.y);
		// ImGui::Text("Selected Object: %d", selectedObjectIndex);

		ImGui::End();


        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");

        m_ViewportWidth = ImGui::GetContentRegionAvail().x;
        m_ViewportHeight = ImGui::GetContentRegionAvail().y;

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

		if (imageMousePos.x > -1 && ImGui::IsMouseDown(0))
		{
			selectedObjectIndex = m_Renderer.ClickQueryObject(imageMousePos.x, finalImage->GetHeight() - imageMousePos.y).ObjectIndex;
		}

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
private:
	bool realtime = true;
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	float m_LastRenderTime = 0.0f;
	glm::vec2 imageMousePos = { -1, -1 };
	int selectedObjectIndex = -1;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
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