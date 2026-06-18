# NPC Equipment Viewer

Plugin SKSE para Skyrim Special Edition/Anniversary Edition.

Ao colocar um NPC no crosshair e pressionar a tecla configurada, o mod abre um menu Scaleform próprio com as armaduras, roupas, escudos e acessórios equipados pelo NPC. Ao confirmar um item, o mod adiciona automaticamente uma regra ao arquivo `Custom_modesty_KID.ini`.

## Recursos

- alvo identificado pelo crosshair;
- tecla configurável por arquivo INI;
- menu Scaleform próprio, sem depender do UIExtensions;
- lista vertical navegável por teclado ou controle;
- dez itens visíveis por página, com rolagem automática;
- texto centralizado em toda a largura da janela;
- redução automática do tamanho da fonte quando um nome realmente excede a largura disponível;
- exibição configurável de tipo, slots e FormID;
- roupas, armaduras, escudos e acessórios equipados;
- slots de armadura entre 30 e 61;
- criação automática de `Custom_modesty_KID.ini`;
- regras KID usando FormID local e plugin de origem;
- comentário com nome e FormID antes de cada regra;
- prevenção de entradas duplicadas;
- sem ESP;
- sem scripts Papyrus;
- sem UIExtensions.

## Dependências

- Skyrim Script Extender, correspondente à versão do Skyrim;
- Address Library for SKSE Plugins, correspondente à versão do Skyrim.

## Estrutura instalada

```text
Data/
├── Interface/
│   └── NPCEquipmentViewerMenu.swf
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

1. Inicie o Skyrim pelo SKSE.
2. Coloque um NPC no centro do crosshair.
3. Pressione `H`.
4. Navegue usando as setas, o direcional ou o analógico do controle.
5. Use `Page Up`, `Page Down`, esquerda ou direita para avançar mais rapidamente.
6. Pressione `Enter` ou o botão de confirmação do controle para escolher o item.
7. Escolha a keyword que será gravada para o item selecionado.
8. Pressione `Esc` ou o botão de voltar para fechar.
9. Reinicie o Skyrim para o Keyword Item Distributor processar uma nova regra.

Com a configuração padrão, cada item é exibido assim:

```text
Apprentice Robes of Conjuration & Illusion | Slot:32
```

## Formato gerado

Exemplo:

```ini
; Steel Armor | ID:0x013955
; NoModestyAll: fully covered
Keyword = NoModestyAll|Armor|0x013955~Skyrim.esm
```

O comentário facilita a identificação manual do item. A regra usa o FormID local e o plugin de origem, portanto não depende do idioma usado pelo jogo.

Keywords disponíveis:

- `Modesty`: top and bottom exposed;
- `NoModesty`: nothing exposed;
- `NoModestyTop`: only top exposed;
- `NoModestyBottom`: only bottom exposed;
- `NoModestyAll`: fully covered.

Caso o arquivo ainda não exista, ele será criado automaticamente. Uma regra já existente para a mesma keyword não será adicionada novamente. Se o item já existir com outra keyword, a regra será atualizada para a nova seleção.

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

As opções abaixo aceitam `true` ou `false`:

- `ShowFormID`: exibe o FormID do equipamento;
- `ShowSlots`: exibe os slots usando o texto `Slot:`;
- `ShowItemType`: exibe o tipo detectado entre colchetes.

O Skyrim deve ser reiniciado depois de alterar o INI.

## Menu Scaleform

O menu é compilado a partir destes arquivos:

```text
interface/
├── NPCEquipmentViewerMenu.xml
└── as2/
    └── NPCEquipmentViewerMenu.as
```

O GitHub Actions gera:

```text
Interface/NPCEquipmentViewerMenu.swf
```

A largura do campo de texto, o alinhamento, a seleção, o fundo e a redução automática da fonte pertencem ao próprio menu. Nenhum elemento interno do `listmenu.swf` do UIExtensions é alterado.

## Log de diagnóstico

O plugin inicializa explicitamente o sistema de log e tenta criar `NPCEquipmentViewer.log` nesta ordem:

```text
Documents\My Games\Skyrim Special Edition\SKSE\NPCEquipmentViewer.log
Data\SKSE\Plugins\NPCEquipmentViewer.log
Skyrim Special Edition\NPCEquipmentViewer.log
```

As mensagens relacionadas ao menu usam o prefixo:

```text
[MenuDiagnostic]
```

O log informa os equipamentos encontrados, as linhas enviadas ao SWF, o retorno do ActionScript e o motivo exato quando alguma etapa falhar.

## Compilação no Windows

### Requisitos da DLL

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

### Compilar a DLL

Abra o **Developer PowerShell for VS 2022**, entre na pasta do projeto e execute:

```powershell
.\build-release.bat
```

A DLL e o INI serão criados em:

```text
build\release\package\SKSE\Plugins
```

O workflow do GitHub também compila o SWF e monta automaticamente o ZIP completo.
