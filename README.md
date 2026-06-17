# NPC Equipment Viewer

Plugin SKSE para Skyrim Special Edition/Anniversary Edition.

Ao colocar um NPC no crosshair e pressionar a tecla configurada, o mod abre uma lista vertical com as armaduras, roupas, escudos e acessórios equipados pelo NPC. Ao confirmar um item, o mod adiciona automaticamente uma regra ao arquivo `Custom_modesty_KID.ini`.

## Recursos

- alvo identificado pelo crosshair;
- tecla configurável por arquivo INI;
- lista vertical navegável por teclado ou controle;
- exibição simplificada com nome e slots do equipamento;
- lista ampliada em 50% para facilitar a leitura;
- roupas, armaduras, escudos e acessórios equipados;
- slots de armadura entre 30 e 61;
- criação automática de `Custom_modesty_KID.ini`;
- regras KID usando FormID local e plugin de origem;
- comentário com nome e FormID antes de cada regra;
- prevenção de entradas duplicadas;
- sem ESP.

## Dependências para usar o mod compilado

- Skyrim Script Extender, correspondente à versão do Skyrim;
- Address Library for SKSE Plugins, correspondente à versão do Skyrim;
- UIExtensions, usado para exibir a lista vertical navegável.

O projeto usa CommonLibSSE-NG, cuja proposta é gerar um único plugin para SE/AE/VR. O código desta versão foi pensado para Skyrim SE/AE; Skyrim VR não foi validado.

## Estrutura instalada

```text
Data/
├── Scripts/
│   ├── NPCEquipmentViewerMenu.pex
│   └── NPCEquipmentViewerNative.pex
└── SKSE/
    └── Plugins/
        ├── NPCEquipmentViewer.dll
        └── NPCEquipmentViewer.ini
```

O arquivo gerado durante o uso fica em:

```text
Data/Custom_modesty_KID.ini
```

## Uso

1. Instale e ative o UIExtensions.
2. Inicie o Skyrim pelo SKSE.
3. Coloque um NPC no centro do crosshair.
4. Pressione `H`.
5. Navegue verticalmente pelos equipamentos usando o teclado ou controle.
6. Pressione `Enter` ou o botão de confirmação do controle para adicionar o item ao KID.
7. Reinicie o Skyrim para o Keyword Item Distributor processar a nova regra.

Cada item é exibido neste formato:

```text
DXFII WildDreams All but Bra_Panty | S:32
```

## Formato gerado

Exemplo:

```ini
; Steel Armor | ID:0x013955
Keyword = NoModestyAll|Armor|0x013955~Skyrim.esm
```

O comentário facilita a identificação manual do item. A regra usa o FormID local e o plugin de origem, portanto não depende do idioma usado pelo jogo.

Caso o arquivo ainda não exista, ele será criado automaticamente. Uma regra já existente não será adicionada novamente.

## Configuração

Arquivo:

```text
Data/SKSE/Plugins/NPCEquipmentViewer.ini
```

Configuração padrão:

```ini
[General]
KeyCode=0x23
ShowFormID=false
ShowSlots=true
ShowItemType=false
```

`0x23` é o código DirectInput da tecla `H`. Consulte `KEYCODES_PT-BR.md` para outros códigos.

A lista sempre exibe somente o nome do item e seus slots. As opções antigas de tipo e FormID foram mantidas no arquivo apenas por compatibilidade.

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
