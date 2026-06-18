class NPCEquipmentViewerMenu
{
    private var root:MovieClip;
    private var panel:MovieClip;
    private var titleField:TextField;
    private var footerField:TextField;

    private var rowBackground0:MovieClip;
    private var rowBackground1:MovieClip;
    private var rowBackground2:MovieClip;
    private var rowBackground3:MovieClip;
    private var rowBackground4:MovieClip;
    private var rowBackground5:MovieClip;
    private var rowBackground6:MovieClip;
    private var rowBackground7:MovieClip;
    private var rowBackground8:MovieClip;
    private var rowBackground9:MovieClip;

    private var rowField0:TextField;
    private var rowField1:TextField;
    private var rowField2:TextField;
    private var rowField3:TextField;
    private var rowField4:TextField;
    private var rowField5:TextField;
    private var rowField6:TextField;
    private var rowField7:TextField;
    private var rowField8:TextField;
    private var rowField9:TextField;

    private static var PANEL_X:Number = 70;
    private static var PANEL_Y:Number = 105;
    private static var PANEL_WIDTH:Number = 1140;
    private static var PANEL_HEIGHT:Number = 510;
    private static var CONTENT_X:Number = 94;
    private static var CONTENT_WIDTH:Number = 1092;
    private static var TITLE_Y:Number = 125;
    private static var ROW_START_Y:Number = 183;
    private static var ROW_HEIGHT:Number = 36;
    private static var FOOTER_Y:Number = 567;

    function NPCEquipmentViewerMenu(rootClip:MovieClip)
    {
        root = rootClip;
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
        row9:String):Number
    {
        SetFittedText(titleField, title, 28, 20, 0xFFFFFF);

        var renderedRows:Number = 0;
        var errorCode:Number = 0;
        var result:Number;

        result = RenderRow(rowBackground0, rowField0, row0, 0, selectedRow);
        if (result < 0) {
            errorCode = result;
        } else {
            renderedRows = renderedRows + result;
        }

        result = RenderRow(rowBackground1, rowField1, row1, 1, selectedRow);
        if (result < 0 && errorCode == 0) {
            errorCode = result;
        } else if (result > 0) {
            renderedRows = renderedRows + result;
        }

        result = RenderRow(rowBackground2, rowField2, row2, 2, selectedRow);
        if (result < 0 && errorCode == 0) {
            errorCode = result;
        } else if (result > 0) {
            renderedRows = renderedRows + result;
        }

        result = RenderRow(rowBackground3, rowField3, row3, 3, selectedRow);
        if (result < 0 && errorCode == 0) {
            errorCode = result;
        } else if (result > 0) {
            renderedRows = renderedRows + result;
        }

        result = RenderRow(rowBackground4, rowField4, row4, 4, selectedRow);
        if (result < 0 && errorCode == 0) {
            errorCode = result;
        } else if (result > 0) {
            renderedRows = renderedRows + result;
        }

        result = RenderRow(rowBackground5, rowField5, row5, 5, selectedRow);
        if (result < 0 && errorCode == 0) {
            errorCode = result;
        } else if (result > 0) {
            renderedRows = renderedRows + result;
        }

        result = RenderRow(rowBackground6, rowField6, row6, 6, selectedRow);
        if (result < 0 && errorCode == 0) {
            errorCode = result;
        } else if (result > 0) {
            renderedRows = renderedRows + result;
        }

        result = RenderRow(rowBackground7, rowField7, row7, 7, selectedRow);
        if (result < 0 && errorCode == 0) {
            errorCode = result;
        } else if (result > 0) {
            renderedRows = renderedRows + result;
        }

        result = RenderRow(rowBackground8, rowField8, row8, 8, selectedRow);
        if (result < 0 && errorCode == 0) {
            errorCode = result;
        } else if (result > 0) {
            renderedRows = renderedRows + result;
        }

        result = RenderRow(rowBackground9, rowField9, row9, 9, selectedRow);
        if (result < 0 && errorCode == 0) {
            errorCode = result;
        } else if (result > 0) {
            renderedRows = renderedRows + result;
        }

        var currentIndex:Number =
            totalCount > 0 ? firstIndex + selectedRow + 1 : 0;

        var footer:String =
            String(currentIndex) +
            " / " +
            String(totalCount) +
            "    Up / Down: Navigate    Enter / Confirm: Add    Esc / Back: Close";

        SetFittedText(footerField, footer, 19, 15, 0xD8D8D8);

        if (errorCode < 0) {
            ShowRenderError(errorCode);
            return errorCode;
        }

        if (renderedRows <= 0 && totalCount > 0) {
            ShowRenderError(-9000);
            return -9000;
        }

        return renderedRows;
    }

    private function RenderRow(
        background:MovieClip,
        field:TextField,
        rowText:String,
        rowIndex:Number,
        selectedRow:Number):Number
    {
        if (background == null) {
            return -1000 - rowIndex;
        }

        if (field == null) {
            return -2000 - rowIndex;
        }

        background.clear();
        background._visible = false;
        field.text = "";
        field._visible = false;

        if (rowText == null || rowText.length <= 0) {
            return 0;
        }

        var selected:Boolean = rowIndex == selectedRow;

        background._visible = true;
        field._visible = true;

        DrawRowBackground(
            background,
            rowIndex,
            selected);

        SetFittedText(
            field,
            rowText,
            selected ? 26 : 24,
            15,
            selected ? 0xFFCC00 : 0xFFFFFF);

        return 1;
    }

    private function ShowRenderError(errorCode:Number):Void
    {
        if (rowBackground0 == null || rowField0 == null) {
            return;
        }

        rowBackground0._visible = true;
        rowField0._visible = true;

        DrawRowBackground(
            rowBackground0,
            0,
            false);

        SetFittedText(
            rowField0,
            "Equipment rows could not be rendered. Error " +
                String(errorCode) +
                ". Check NPCEquipmentViewer.log.",
            22,
            14,
            0xFFAAAA);
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

        rowBackground0 = CreateRowBackground("rowBackground0");
        rowBackground1 = CreateRowBackground("rowBackground1");
        rowBackground2 = CreateRowBackground("rowBackground2");
        rowBackground3 = CreateRowBackground("rowBackground3");
        rowBackground4 = CreateRowBackground("rowBackground4");
        rowBackground5 = CreateRowBackground("rowBackground5");
        rowBackground6 = CreateRowBackground("rowBackground6");
        rowBackground7 = CreateRowBackground("rowBackground7");
        rowBackground8 = CreateRowBackground("rowBackground8");
        rowBackground9 = CreateRowBackground("rowBackground9");

        rowField0 = CreateRowField("rowField0", 0);
        rowField1 = CreateRowField("rowField1", 1);
        rowField2 = CreateRowField("rowField2", 2);
        rowField3 = CreateRowField("rowField3", 3);
        rowField4 = CreateRowField("rowField4", 4);
        rowField5 = CreateRowField("rowField5", 5);
        rowField6 = CreateRowField("rowField6", 6);
        rowField7 = CreateRowField("rowField7", 7);
        rowField8 = CreateRowField("rowField8", 8);
        rowField9 = CreateRowField("rowField9", 9);

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

    private function CreateRowBackground(name:String):MovieClip
    {
        var background:MovieClip = root.createEmptyMovieClip(
            name,
            root.getNextHighestDepth());
        background._visible = false;
        return background;
    }

    private function CreateRowField(
        name:String,
        rowIndex:Number):TextField
    {
        var rowY:Number =
            ROW_START_Y + (rowIndex * ROW_HEIGHT);

        root.createTextField(
            name,
            root.getNextHighestDepth(),
            CONTENT_X,
            rowY,
            CONTENT_WIDTH,
            ROW_HEIGHT);

        var field:TextField = TextField(root[name]);
        ConfigureTextField(field);
        field._visible = false;
        return field;
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
        background._visible = false;
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

    private function DrawRectangleOutline(
        target:MovieClip,
        x:Number,
        y:Number,
        width:Number,
        height:Number,
        color:Number,
        alpha:Number,
        thickness:Number):Void
    {
        target.lineStyle(thickness, color, alpha);
        target.moveTo(x, y);
        target.lineTo(x + width, y);
        target.lineTo(x + width, y + height);
        target.lineTo(x, y + height);
        target.lineTo(x, y);
    }
}
