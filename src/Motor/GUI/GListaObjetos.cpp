#include "GListaObjetos.hpp"
#include "../Camaras/CamarasGestor.hpp"
#include "../Primitivos/GestorEscenas.hpp"
#include <imgui.h>
#include <imgui-SFML.h>

namespace CE
{
    void GListaObjetos::OnInit(const MotorConfig& des)
    {
        propiedades.OnInit(des);
    }
    void GListaObjetos::OnUpdate(float dt)
    {
        propiedades.OnUpdate(dt);
    }
    void GListaObjetos::OnRender(void)
    {
        ImGui::Begin("Objetos",nullptr,0);
        
        static int id_escena_actual =0;
        auto lista = GestorEscenas::Get().getKeys();
        const char* str_escena = GestorEscenas::Get().getEscenaActual().nombre.c_str();
        if(ImGui::BeginCombo("Escenas",str_escena,0))
        {
            for(int i=0;i<lista.size();++i)
            {
                const bool seleccion = (id_escena_actual==i);
                if(ImGui::Selectable(lista[i].c_str(),seleccion))
                    id_escena_actual=i;

                if(seleccion)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            GestorEscenas::Get().cambiarEscena(lista[id_escena_actual]);

            ImGui::EndCombo();
        }

        ImGui::Spacing();
        if(ImGui::TreeNode("Camaras"))
        {
            auto camaras = GestorCamaras::Get().getListaCamaras();
            for(int i=0; i<camaras.size();i++)
            {
                auto cam = camaras[i];
                auto strCam = cam->nombre;
                //color del boton
                if(cam->esta_activa)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button,0xff00A5FF);
                    ImGui::PushStyleColor(ImGuiCol_Text,0xff000000);
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_Button,0xffffbf00);
                    ImGui::PushStyleColor(ImGuiCol_Text,0xff000000);
                }

                bool boton = ImGui::Button(strCam.c_str(),ImVec2(150,30));
                if(boton)
                {
                    GestorCamaras::Get().setCamaraActiva(i);
                }
                ImGui::PopStyleColor(2);
            }
            ImGui::TreePop();
            ImGui::Spacing();
        }
        if(ImGui::TreeNode("Objetos"))
        {
            auto lista = GestorEscenas::Get().getEscenaActual().getPool();
            for(auto &obj : lista.getPool())
            {
                if(ImGui::Button(obj->toString().c_str()))
                {
                    GestorCamaras::Get().getCamaraActiva().lockEnObjeto(obj);
                    propiedades.seleccionarObjeto(obj.get());
                }

            }
            ImGui::TreePop();
        }
        ImGui::End();

        propiedades.OnRender();

    }
}
