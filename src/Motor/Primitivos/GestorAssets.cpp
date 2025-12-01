#include "GestorAssets.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>

namespace CE
{
    GestorAssets& GestorAssets::Get()
    {
        if(!instancia)
            instancia = new GestorAssets();
        return *instancia;
    }
    void GestorAssets::agregarFont(const std::string& key, const std::string& filepath)
    {
        auto font = std::make_shared<sf::Font>();
        if(!font->openFromFile(filepath))
        {
            std::cerr<<"No se pudo abrir el archivo "<<filepath<<"\n";
            exit(EXIT_FAILURE);
        }

        hashFonts[key] = font;

    }
    void GestorAssets::agregarTextura(const std::string& key, const std::string& filepath,
            const CE::Vector2D& pos_init,const CE::Vector2D& dim)
    {
        auto textura = std::make_shared<sf::Texture>();
        if(!textura->loadFromFile(filepath,false,
                    sf::IntRect({(int)pos_init.x,(int)pos_init.y},{(int)dim.x,(int)dim.y})
                    ))
        {
            std::cerr<<"No se pudo cargar la textura "<<filepath<<"\n";
            exit(EXIT_FAILURE);
        }

        textura->setSmooth(true);
        hashTexturas[key] = textura;
    }

    void GestorAssets::agregarSonido(const std::string& key, const std::string& filepath)
    {
        if(!soundBuffer.loadFromFile(filepath))
            std::cerr<<"No se pudo cargar la Sonido "<<filepath<<"\n";
        hashSonidos[key] = std::make_shared<sf::Sound>(soundBuffer);
    }

    void GestorAssets::agregarMusica(const std::string& key, const std::string& filepath)
    {
        auto musica = std::make_shared<sf::Music>();
        if(!musica->openFromFile(filepath))
            std::cerr<<"No se pudo cargar la Música "<<filepath<<"\n";
        hashMusica[key] = musica;
    }

    void GestorAssets::cargarDialogues(const std::string& filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cerr << "No se pudo abrir el archivo de diálogos: " << filepath << "\n";
            return;
        }

        std::string line;
        // Skip header line (ID | TEXT)
        std::getline(file, line);

        // UTF-8 to wide string converter
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

        while (std::getline(file, line))
        {
            if (line.empty())
                continue;

            // Find the delimiter " | "
            size_t delimiter_pos = line.find(" | ");
            if (delimiter_pos == std::string::npos)
                continue;

            // Extract ID and TEXT
            std::string id_str = line.substr(0, delimiter_pos);
            std::string text_str = line.substr(delimiter_pos + 3); // Skip " | "

            try
            {
                int id = std::stoi(id_str);
                std::wstring text = converter.from_bytes(text_str);
                dialogues[id] = text;
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error parsing dialogue line: " << line << "\n";
                std::cerr << "Exception: " << e.what() << "\n";
            }
        }

        file.close();
        std::cout << "Loaded " << dialogues.size() << " dialogues from " << filepath << "\n";
    }

    const std::wstring& GestorAssets::getDialogue(int id) const
    {
        static const std::wstring empty_dialogue = L"[Diálogo no encontrado]";

        auto it = dialogues.find(id);
        if (it != dialogues.end())
        {
            return it->second;
        }

        std::cerr << "Warning: Dialogue ID " << id << " not found\n";
        return empty_dialogue;
    }
}
