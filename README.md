# NPC Equipment Viewer

Plugin SKSE para Skyrim Special Edition/Anniversary Edition.

Ao colocar um NPC no crosshair e pressionar a tecla configurada, o mod abre uma janela nativa do Skyrim mostrando os itens equipados pelo NPC.

## Recursos

- alvo identificado pelo crosshair;
- tecla configurável por arquivo INI;
- mão direita e mão esquerda;
- roupas, armaduras, escudos e acessórios equipados;
- slots de armadura entre 30 e 61;
- FormID opcional;
- sem ESP;
- sem Papyrus;
- sem SkyUI;
- sem UIExtensions.

## Dependências para usar o mod compilado

- Skyrim Script Extender, correspondente à versão do Skyrim;
- Address Library for SKSE Plugins, correspondente à versão do Skyrim.

O projeto usa CommonLibSSE-NG, cuja proposta é gerar um único plugin para SE/AE/VR. O código desta versão foi pensado para Skyrim SE/AE; Skyrim VR não foi validado.

## Estrutura instalada

```text
Data/
└── SKSE/
    └── Plugins/
        ├── NPCEquipmentViewer.dll
        └── NPCEquipmentViewer.ini
```

## Uso

1. Inicie o Skyrim pelo SKSE.
2. Coloque um NPC no centro do crosshair.
3. Pressione `K`.
4. Feche a janela com `Enter`, `Esc` ou o botão exibido pelo jogo.

## Configuração

Arquivo:

```text
Data/SKSE/Plugins/NPCEquipmentViewer.ini
```

Configuração padrão:

```ini
[General]
KeyCode=0x25
ShowFormID=true
ShowSlots=true
ShowItemType=true
```

`0x25` é o código DirectInput da tecla `K`. Consulte `KEYCODES_PT-BR.md` para outros códigos.

O Skyrim deve ser reiniciado depois de alterar o INI.

## Compilação no Windows

### Requisitos

- Visual Studio 2022 com **Desenvolvimento para Desktop com C++**;
- CMake;
- Ninja;
- Git;
- vcpkg.

### Preparar o vcpkg

No PowerShell:

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\Ferramentas\vcpkg
C:\Ferramentas\vcpkg\bootstrap-vcpkg.bat
```

Crie a variável de ambiente:

```text
Nome: VCPKG_ROOT
Valor: C:\Ferramentas\vcpkg
```

Feche e abra novamente o terminal depois de criar a variável.

### Compilar

Abra o **Developer PowerShell for VS 2022**, entre na pasta do projeto e execute:

```powershell
.\build-release.bat
```

O pacote será criado em:

```text
build\release\package
```

## Instalação automática durante a compilação

Para copiar automaticamente o mod para o Mod Organizer 2 ou Vortex, crie a variável:

```text
SKYRIM_MODS_FOLDER=X:\caminho\para\mods
```

O projeto criará a pasta:

```text
NPC Equipment Viewer\SKSE\Plugins
```

Também é possível definir:

```text
SKYRIM_FOLDER=X:\SteamLibrary\steamapps\common\Skyrim Special Edition
```

Nesse caso, os arquivos serão copiados diretamente para a pasta `Data` do jogo.

## Limitação visual desta versão

A lista é exibida por uma caixa de diálogo nativa do Skyrim. Ela é somente leitura. Uma versão posterior pode trocar essa janela por um menu Scaleform próprio, com rolagem, ícones e seleção de itens.
