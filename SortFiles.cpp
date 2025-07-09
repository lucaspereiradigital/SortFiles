#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

static void showHelp();
static void sortFiles(fs::path const& folderPath, bool const& recurse);

int wmain(int argc, wchar_t** argv)
{
    std::setlocale(LC_ALL, "pt_BR.UTF-8");

    // Sem argumentos = Mostrar mensagem de ajuda
    if (argc == 1)
    {
        showHelp();
        return 0;
    }

    bool recurse = false;

    // Se o nome do arquivo termina com "++" ou se o parâmetro --recurse* foi passado, a recursão é ativada.
    if (std::wstring(argv[0]).ends_with(L"++.exe"))
    {
        recurse = true;
    }
    else
    {
        for (int i = 1; i < argc; ++i)
        {
            if (std::wstring(argv[i]) == L"--recurse*")
            {
                recurse = true;
                break;
            }
        }
    }

    // Percorre os caminhos de pasta informados, processando-os e passando-os para o método de organização
    for (int i = 1; i < argc; ++i)
    {
        std::wstring arg(fs::absolute(argv[i]));

        if (!arg.starts_with(L"\\\\?\\"))
        {
            arg = L"\\\\?\\" + arg;
        }

        if (fs::is_directory(arg))
        {
            sortFiles(arg, recurse);
        }
        else if (arg.ends_with(L'*'))
        {
            arg = arg.substr(0, arg.length() - 1);

            if (fs::is_directory(arg))
            {
                sortFiles(arg, true);
            }
        }
    }

    return 0;
}

// Cria e exibe a mensagem de ajuda
static void showHelp()
{
    char const nl = '\n';

    std::cout <<
        "Organiza arquivos com base em sua extensão." << nl <<
        nl <<
        "USO:" << nl <<
        "   SortFiles <caminho da pasta 1>[*] [<caminho da pasta 2>[*]] ... [<caminho da pasta n>[*]] [--recurse*]" << nl <<
        nl <<
        "EXEMPLOS:" << nl <<
        "   SortFiles \"C:\\Users\\Joao\\Downloads\\Meus Jogos*\" C:\\Users\\Joao\\Pictures\\FotosCasamento" << nl <<
        "   SortFiles Recordacoes\\MinhasFotos \"C:\\Users\\Maria\\Documents\\Meu Projeto\" --recurse*" << nl <<
        nl <<
        "DESCRIÇÃO:" << nl <<
        "   Para cada extensão de arquivo encontrada, será criada uma pasta com o nome da extensão e os arquivos serão movidos para " <<
        "essas pastas." << nl <<
        nl <<
        "   - Caminhos podem ser relativos ou absolutos." << nl <<
        "   - Caminhos com espaços ou símbolos especiais devem estar entre aspas duplas." << nl <<
        "   - Caminhos inexistentes são ignorados." << nl <<
        "   - Suporta caminhos longos (acima de 260 caracteres no Windows)." << nl <<
        "   - Suporta arrasto de pastas em cima do executável." << nl <<
        nl <<
        "IMPORTANTE:" << nl <<
        "   Arquivos cuja extensão seja igual ao nome da pasta onde estão só serão movidos se a pasta de destino com o mesmo nome da " <<
        "extensão já existir." << nl <<
        "   Se a pasta de destino já contiver um arquivo com o mesmo nome, o arquivo não será movido." << nl <<
        "   Se a pasta de origem contiver um arquivo sem extensão cujo nome conflite com o nome de uma pasta de destino que seria " <<
        "criada, os arquivos que seriam organizados nessa pasta serão ignorados." << nl <<
        nl <<
        "RECURSIVIDADE:" << nl <<
        "   É possível organizar também os arquivos de subpastas:" << nl <<
        nl <<
        "   - Usando o parâmetro global: --recurse*" << nl <<
        "   - Renomeando o executável para conter \"++\" no final do nome: SortFiles++.exe" << nl <<
        "   - Adicionando \"*\" ao fim do caminho: Caminho\\Para\\A\\Pasta* ou Caminho\\Para\\A\\Pasta\\*" << nl <<
        std::flush;

    return;
}

// Organiza os arquivos contidos no caminho de pasta recebido pelo parâmetro folderPath
// e de suas subpastas caso a recursão tenha sido solicitada
static void sortFiles(fs::path const& folderPath, bool const& recurse)
{
    for (fs::directory_entry const& item : fs::directory_iterator(folderPath, fs::directory_options::skip_permission_denied))
    {
        if (item.is_directory() && recurse)
        {
            sortFiles(item.path(), recurse);
        }
        else if (item.is_regular_file())
        {
            fs::path folderName;
            fs::path ext(item.path().extension());

            if (ext.empty())
            {
                folderName = L"no_extension";
            }
            else
            {
                folderName = ext.wstring().substr(1);
            }

            fs::path const extFolderPath(folderPath / folderName);

            // Cria o diretório com o nome da extensão do arquivo a ser organizado, caso ainda não exista
            if (!fs::is_directory(extFolderPath))
            {
                // Caso exista um arquivo sem extensão cujo nome seja igual ao da pasta que seria criada,
                // o arquivo não é organizado.
                // Caso o arquivo a ser organizado esteja numa pasta cujo nome seja igual ao de sua extensão,
                // o arquivo já está organizado (verificação útil pra evitar possível organização repetida em
                // caso de recursão).
                if (fs::is_regular_file(extFolderPath) || item.path().parent_path().filename() == folderName)
                {
                    continue;
                }

                fs::create_directory(extFolderPath);
            }

            fs::path const filePath(extFolderPath / item.path().filename());

            if (!fs::exists(filePath))
            {
                fs::rename(item.path(), filePath);
            }
        }
    }
}
