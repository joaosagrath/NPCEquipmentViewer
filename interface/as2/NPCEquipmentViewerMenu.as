class NPCEquipmentViewerMenu
{
    private var root:MovieClip;
    private var panel:MovieClip;
    private var titleField:TextField;
    private var footerField:TextField;
    private var rowBackgrounds:Array;
    private var rowFields:Array;

    private static var PANEL_X:Number = 70;
    private static var PANEL_Y:Number = 105;
    private static var PANEL_WIDTH:Number = 1140;
    private static var PANEL_HEIGHT:Number = 510;
    private static var CONTENT_X:Number = 94;
    private static var CONTENT_WIDTH:Number = 1092;
    private static var TITLE_Y:Number = 125;
    private static var ROW_START_Y:Number = 183;
    private static var ROW_HEIGHT:Number = 36;
    private static var ROW_COUNT:Number = 10;
    private static var FOOTER_Y:Number = 567;

    function NPCEquipmentViewerMenu(rootClip:MovieClip)
    {
        root = rootClip;
        rowBackgrounds = [];
        rowFields = [];
        BuildInterface();
    }

    static function main():Void
    {
        _global.gfxExtensions = true;
        Stage.scaleMode = "noScale";
        Stage.align = "TL";

        var menu:NPCEquipmentViewerMenu =
            new NPCEquipmentViewerMenu(_root);
        _root.NPCEquipmentViewerMenu = menu;
    }

    public function SetData(
        title:String,
        selectedRow:Number,
        firstIndex:Number,
        totalCount:Number,
        row0:String,
        row1:String,
        row2:String,
        row3:String,
        row4:String,
        row5:String,
        row6:String,
        row7:String,
        row8:String,
        row9:String,
        visibleRowCount:Number):Number
    {
        var rows:Array = [
            row0,
            row1,
            row2,
            row3,
            row4,
            row5,
            row6,
            row7,
            row8,
            row9
        ];

        SetFittedText(titleField, title, 28, 20, 0xFFFFFF);

        var renderedRows:Number = 0;
        var index:Number;

        for (index = 0; index < ROW_COUNT; index++) {
            var background:MovieClip =
                MovieClip(rowBackgrounds[index]);
            var field:TextField =
                TextField(rowFields[index]);

            if (background != null) {
                background.clear();
                background._visible = false;
            }

            if (field != null) {
                field.text = "";
                field._visible = false;
            }

            if (
                index < visibleRowCount &&
                background != null &&
                field != null) {
                background._visible = true;
                field._visible = true;

                DrawRowBackground(
                    background,
                    index,
                    index == selectedRow);

                SetFittedText(
                    field,
                    String(rows[index]),
                    24,
                    15,
                    0xFFFFFF);

                renderedRows = renderedRows + 1;
            }
        }

        if (renderedRows <= 0 && visibleRowCount > 0) {
            var fallbackBackground:MovieClip =
                MovieClip(rowBackgrounds[0]);
            var fallbackField:TextField =
                TextField(rowFields[0]);

            if (fallbackBackground != null && fallbackField != null) {
                fallbackBackground._visible = true;
                fallbackField._visible = true;

                DrawRowBackground(
                    fallbackBackground,
                    0,
                    false);

                SetFittedText(
                    fallbackField,
                    "No equipment rows were rendered. Check NPCEquipmentViewer.log.",
                    22,
                    14,
                    0xFFAAAA);
            }
        }

        var currentIndex:Number =
            totalCount > 0 ? firstIndex + selectedRow + 1 : 0;

        var footer:String =
            String(currentIndex) +
            " / " +
            String(totalCount) +
            "    Up / Down: Navigate    Enter / Confirm: Add    Esc / Back: Close";

        SetFittedText(footerField, footer, 19, 15, 0xD8D8D8);

        return renderedRows;
    }

    private function BuildInterface():Void
    {
        panel = root.createEmptyMovieClip(
            "panel",
            root.getNextHighestDepth());

        DrawRectangle(
            panel,
            PANEL_X,
            PANEL_Y,
            PANEL_WIDTH,
            PANEL_HEIGHT,
            0x000000,
            78);

        var topLine:MovieClip = root.createEmptyMovieClip(
            "topLine",
            root.getNextHighestDepth());
        DrawRectangle(
            topLine,
            PANEL_X,
            PANEL_Y,
            PANEL_WIDTH,
            2,
            0xFFFFFF,
            65);

        var bottomLine:MovieClip = root.createEmptyMovieClip(
            "bottomLine",
            root.getNextHighestDepth());
        DrawRectangle(
            bottomLine,
            PANEL_X,
            PANEL_Y + PANEL_HEIGHT - 2,
            PANEL_WIDTH,
            2,
            0xFFFFFF,
            65);

        root.createTextField(
            "titleField",
            root.getNextHighestDepth(),
            CONTENT_X,
            TITLE_Y,
            CONTENT_WIDTH,
            42);
        titleField = TextField(root["titleField"]);
        ConfigureTextField(titleField);

        var index:Number;
        for (index = 0; index < ROW_COUNT; index++) {
            var rowY:Number =
                ROW_START_Y + (index * ROW_HEIGHT);

            var background:MovieClip =
                root.createEmptyMovieClip(
                    "rowBackground" + index,
                    root.getNextHighestDepth());
            background._visible = false;
            rowBackgrounds.push(background);

            root.createTextField(
                "rowField" + index,
                root.getNextHighestDepth(),
                CONTENT_X,
                rowY,
                CONTENT_WIDTH,
                ROW_HEIGHT);
            var field:TextField =
                TextField(root["rowField" + index]);
            ConfigureTextField(field);
            field._visible = false;
            rowFields.push(field);
        }

        root.createTextField(
            "footerField",
            root.getNextHighestDepth(),
            CONTENT_X,
            FOOTER_Y,
            CONTENT_WIDTH,
            30);
        footerField = TextField(root["footerField"]);
        ConfigureTextField(footerField);
    }

    private function ConfigureTextField(field:TextField):Void
    {
        field.selectable = false;
        field.multiline = false;
        field.wordWrap = false;
        field.autoSize = "none";
        field.embedFonts = false;
        field.background = false;
        field.border = false;
        field._visible = true;
    }

    private function DrawRowBackground(
        background:MovieClip,
        index:Number,
        selected:Boolean):Void
    {
        background.clear();

        var rowY:Number =
            ROW_START_Y + (index * ROW_HEIGHT);

        if (selected) {
            DrawRectangle(
                background,
                CONTENT_X,
                rowY,
                CONTENT_WIDTH,
                ROW_HEIGHT - 2,
                0xFFFFFF,
                24);
        } else {
            DrawRectangle(
                background,
                CONTENT_X,
                rowY,
                CONTENT_WIDTH,
                ROW_HEIGHT - 2,
                0x000000,
                18);
        }
    }

    private function SetFittedText(
        field:TextField,
        value:String,
        maximumSize:Number,
        minimumSize:Number,
        color:Number):Void
    {
        field.text = value;

        var size:Number = maximumSize;
        var format:TextFormat =
            CreateTextFormat(size, color);

        field.setTextFormat(format);
        field.setNewTextFormat(format);

        while (
            field.textWidth > field._width - 12 &&
            size > minimumSize) {
            size = size - 1;
            format = CreateTextFormat(size, color);
            field.setTextFormat(format);
            field.setNewTextFormat(format);
        }
    }

    private function CreateTextFormat(
        size:Number,
        color:Number):TextFormat
    {
        var format:TextFormat =
            new TextFormat(
                "$EverywhereFont",
                size,
                color,
                false,
                false,
                false,
                null,
                null,
                "center");

        format.align = "center";
        return format;
    }

    private function DrawRectangle(
        target:MovieClip,
        x:Number,
        y:Number,
        width:Number,
        height:Number,
        color:Number,
        alpha:Number):Void
    {
        target.beginFill(color, alpha);
        target.moveTo(x, y);
        target.lineTo(x + width, y);
        target.lineTo(x + width, y + height);
        target.lineTo(x, y + height);
        target.lineTo(x, y);
        target.endFill();
    }
}
