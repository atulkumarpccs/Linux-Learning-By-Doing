------------------------------------------------------------------------------------
-- Filename         : KeyBoard.lua
-- Module           : GUI Widgets
-- Project          : Nissan LCN
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : Speller widget.
--
-- Constraints      : none
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
--
--
--  Explanation of some member variables:
--
--
--   * self.nLayout
--
--     Index of the current layout (always a valid number in the range
--     1...#self.tttLayouts)
--
--
--   * self.nCursor
--
--     Index of the cursor (focused) button, or nil if there is no cursor.
--
--
--   * self.bPressed
--
--     true while one of the speller buttons is pressed,
--     false otherwise.
--
--
--   * self.bGotFocus
--
--     true if the speller currently is registered to the DDS events,
--     false otherwise. Initially set to false.
--
--
--   * self.bEnabled
--
--     Reflects the state of the speller property Enabled. If this is
--     false, all letters are disabled but the cursor remains at the
--     previous position. When it gets enabled again, the cursor is
--     repositioned at the closest enabled button.
--
--
--   * self.ttBitmaps
--
--     two-dimensional array with button bitmaps
--
--     First index defines the button state:
--        1 - enabled
--        2 - focused
--        3 - pressed
--        4 - disabled
--
--     Second index defines the button types:
--        1 - Top left Button
--        2 - Top Right Button
--        3 - Bottom Left Button
--        4 - Bottom Right Button
--        5 - Bright Button
--        6 - Dark Button
--
--
--   * self.ttColors
--
--     two-dimensional array with text colors
--     see self.ttBitmaps for explanation of indexes
--
--
--   * self.tttButtons
--
--     Array with arrays of static information about the buttons of a speller screen.
--     There will be one one per layout (QWERTY - 36 buttons,
--                                       NumBlock - 12 buttons
--                                       ABC - 28 buttons)
--     Contains the following information about a button:
--
--        tttButtons[l][b].x         x coordinate (relative to widget x position)
--        tttButtons[l][b].y         y coordinate (relative to widget y position)
--        tttButtons[l][b].nWidth    width
--        tttButtons[l][b].nHeight   height
--        tttButtons[l][b].nType     type (1-6); defines graphical representation
--
--
--   * self.tttLayouts
--
--     Array with layouts, which are defined by an array of button data.
--     This array contains all data defined by the layout sequence and all
--     dynamic data (e.g. enabled flag).
--
--        tttLayouts.bUseFill           true if dynamically filled characters are used
--        tttLayouts[l].bVisible        true if layout contains any buttons with letter
--        tttLayouts[l][b].bFill        true if this character can be dynamically used
--        tttLayouts[l][b].bEnabled     true if this button is enabled
--        tttLayouts[l][b].nCharacter   character code, or 0 if unused
--        tttLayouts[l][b].sText        text for the button
--
--
------------------------------------------------------------------------------------
--
--
--  Description of special focus message (WIDGET_SETFOCUS):
--
--   * Receive message from ButtonContainer
--
--     param 1: link to ButtonContainer
--     param 2: 1 - move focus to first button of speller
--              2 - move focus to last button of speller
--
--   * Send message from Speller to ButtonContainer
--
--     param 1: link to Speller
--     param 2: 1 - move focus to first button of button container
--              2 - move focus to first button of button container,
--                  or use invisible focus if there is no enabled button in the b.c.
--              3 - move focus to button container (invisible focus)
--
--
------------------------------------------------------------------------------------


Speller = defineClass(Scripted)


Speller.QWERTY_LAYOUT = 1
Speller.ABC_LAYOUT = 2
Speller.ABC_LAYOUT_EXT = 3
Speller.NUMBLOCK_LAYOUT = 4
Speller.ABC_LAYOUT_47BTN = 5

------------------------------------------------------------------------------------
-- function         : constructor
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : The Lua constructor is called after all of the C++ widgets
--                    have been created. Therefore, you can access other widgets
--                    (parent, children, siblings), and also get/set all widget
--                    properties.
--
--                    The following variables will be set before calling the
--                    constructor:
--                       - self.cself: pointer to the widget (C++ pointer)
--                       - self.classname: name of the widget class
--                       - self.cidself: content id of a self link
------------------------------------------------------------------------------------
function Speller:constructor()

    -- register data update events
    RegisterDataUpdate(self.cself, GetProperty(self.cself, "GetLetterFunction", GETPROPERTY_DATATYPE_CID), 0)
    RegisterDataUpdate(self.cself, GetProperty(self.cself, "InvertGetLetterFunction", GETPROPERTY_DATATYPE_CID), 0)
    RegisterDataUpdate(self.cself, GetProperty(self.cself, "Enabled", GETPROPERTY_DATATYPE_CID), 0)

    -- register to view-change event
    RegisterEvent(self.cself, GUI_EVENT_WIDGET_VIEW_CHANGE, 10, true)

    -- set some internal variables
    self.x, self.y = GetPosition(self.cself)
    self.nCursor = nil
    self.bGotFocus = false
    self.bPressed = false
    self.nPressed = nil
    self.nLayout = 1
    self.bEnabled = true
    self.bSpaceBtnEnabled = false

    -- get state dependent bitmaps
    self.ttBitmaps = {
        { GetProperty(self.cself, "BmEnabled",  GETPROPERTY_DATATYPE_CID) },
        { GetProperty(self.cself, "BmFocus",    GETPROPERTY_DATATYPE_CID) },
        { GetProperty(self.cself, "BmPressed",  GETPROPERTY_DATATYPE_CID) },
        { GetProperty(self.cself, "BmDisabled", GETPROPERTY_DATATYPE_CID) }
    }

    -- get state dependent colors
    self.ttColors = {
        { GetProperty(self.cself, "ColEnabled" ) },
        { GetProperty(self.cself, "ColFocus"   ) },
        { GetProperty(self.cself, "ColPressed" ) },
        { GetProperty(self.cself, "ColDisabled") }
    }

    -- get other properties
    self.nFontId = GetProperty(self.cself, "Font")
    self.nFontHeight = GetFontHeight(self.cself, self.nFontId)
    self.nToggleLayoutEvent = math._and(GetProperty(self.cself, "IN_EVENT_toggle_layout", GETPROPERTY_DATATYPE_CID) or 0, 0xFFFF)
    self.nBaseline = GetProperty(self.cself, "Baseline")  or  0

    -- register to toggle-layout event
    if self.nToggleLayoutEvent == 0 then
        self.nToggleLayoutEvent = nil
    else
        RegisterEvent(self.cself, self.nToggleLayoutEvent, 10)
    end

    -- define special characters
    self.nEmpty, self.nFill, self.nEol = GetSpellerSpecialCharacters()
    self.nSpace = string.byte(" ")
    --print("special chars " .. tostring(self.nEmpty) .. ", " .. tostring(self.nFill) .. ", " .. tostring(self.nSpace))

    self.nCurrentLayoutType = Speller.QWERTY_LAYOUT
    self.pToggleButton = self:findToggleBtn() or nil
    -- build table with button positions and decode sequence
    self.tttButtons  = { }
    self.tNumButtons = { }
    self:buildButtonTable_QWERTY()
    self:buildButtonTable_ABC(true)
    self:buildButtonTable_ABC(false)
    self:buildButtonTable_ABC_47Btn()
    self:buildButtonTable_NumBlock()
    self:buildLayoutTable()
    
    if #self.tttLayouts[self.nLayout] == #self.tttButtons[Speller.ABC_LAYOUT] then
       self.nCurrentLayoutType = Speller.ABC_LAYOUT
    elseif #self.tttLayouts[self.nLayout] == #self.tttButtons[Speller.ABC_LAYOUT_EXT] then
       self.nCurrentLayoutType = Speller.ABC_LAYOUT_EXT
    elseif #self.tttLayouts[self.nLayout] == #self.tttButtons[Speller.ABC_LAYOUT_47BTN] then
       self.nCurrentLayoutType = Speller.ABC_LAYOUT_47BTN
    elseif #self.tttLayouts[self.nLayout] == #self.tttButtons[Speller.QWERTY_LAYOUT] then
       self.nCurrentLayoutType = Speller.QWERTY_LAYOUT
    elseif #self.tttLayouts[self.nLayout] == #self.tttButtons[Speller.NUMBLOCK_LAYOUT] then
       self.nCurrentLayoutType = Speller.NUMBLOCK_LAYOUT
    else
       TTFis_Trace(self.cself,"Unknown Layout with "..tostring(#self.tttLayouts[self.nLayout]).." characters")
    end

    -- correctly set enabled/disabled buttons
    self:updateEnabled()

end


------------------------------------------------------------------------------------
-- function         : destructor
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
--  constraints     : The widget was registered for an event only in case of
--                    FocusGained or a valid ToggleLayoutEvent.
--                    So a deregistration is done conditionally.
------------------------------------------------------------------------------------
-- History          : Remove all registrations of the widget
------------------------------------------------------------------------------------
function Speller:destructor()

    -- Remove all registrations of the widget
    if self.bGotFocus or self.nToggleLayoutEvent then
        -- Note: Currently the widget was registered for an event only in case
        --       of FocusGained or a valid ToggleLayoutEvent.
        DeregisterEvent(self.cself)
    end
end


------------------------------------------------------------------------------------
-- Function         : findToggleBtn
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : Find the Toggle Layout button
------------------------------------------------------------------------------------
-- History:
------------------------------------------------------------------------------------
function Speller:findToggleBtn()
--Recursive function to find the widgets from the complete tree
    local function checkChild(pParent)
        local nParVisible = IsVisible(pParent) or false
        if nParVisible == false then return end
        local pReturn = nil
        for _, pWidget in ipairs{GetChildWidgets(pParent)} do
            if GetWidgetName(pWidget) == "ToggleLayout" then
                return pWidget
            else
                pReturn = checkChild(pWidget)
                if pReturn ~= nil then break end
            end
        end
        return pReturn
    end

    local nvisible = IsVisible(self.cself) or false

    if nvisible == true then
        local pParent = GetParentWidget(self.cself)
        while GetParentWidget(pParent) do
           pParent = GetParentWidget(pParent)
        end
        return checkChild(pParent)
    end
end
------------------------------------------------------------------------------------
-- function         : buildLayoutTable
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : Build the table tttLayouts. Dynamic data
--                    (depending on API calls) will not be correctly set.
--                    Use updateEnabled() for dynamic data.
------------------------------------------------------------------------------------
function Speller:buildLayoutTable()
   
    -- build table self.tttLayouts
    local sSequenceName = GetProperty(self.cself, "LayoutSequence", GETPROPERTY_DATATYPE_STRING) or ""
    local tsSequence = { GetSpellerLayoutSequence(sSequenceName) }
    --for n, seq in ipairs(tsSequence) do print("sequence " .. sSequenceName .. " layout " .. n .. ": " .. seq) end
    local sSpace = GetProperty(self.cself, "TextSpace", GETPROPERTY_DATATYPE_STRING)

    self.arabicString = { }
    local function CB(txt)
        self.arabicString[0] = { txt=txt }
    end
    
    FormatArabic(CB,sSpace,0)

    self.tttLayouts = { bUseFill = false }
    for i = 1, math.max(#tsSequence, 1) do
        local ttLayout = { bVisible = false }
        local tCharacters = { UTF8Decode(tsSequence[i] or "") }
        
        for j = 1, #tCharacters do
            local nCharacter = tCharacters[j] or 0
            local sText
            local bFill = nCharacter == self.nFill
            self.tttLayouts.bUseFill = self.tttLayouts.bUseFill or bFill
            if nCharacter == self.nEmpty or nCharacter == 0 or bFill then
                nCharacter = 0
                sText = ""
                TTFis_Trace(self.cself,"Speller:buildLayoutTable  empty character"..tostring(j))
            else
                if nCharacter == self.nSpace then
                    sText = self.arabicString[0].txt
                elseif nCharacter == self.nEol then
                    nCharacter = 0x000A
                    sText = UTF8Encode(nCharacter)
                else
                    sText = UTF8Encode(nCharacter)
                end
            end
            
            ttLayout[j] = {
                bFill      = bFill,
                nCharacter = nCharacter,
                sText      = sText,
                bEnabled   = false
            }
        end
        self.tttLayouts[i] = ttLayout
    end
end

------------------------------------------------------------------------------------
-- function         : buildButtonTable_ABC
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : Build a table for the ABC layout having 28 buttons and adds it
--                    to self.tttButtons. Dynamic data
--                    (depending on API calls) will not be correctly set.
--                    Use updateEnabled() for dynamic data.
------------------------------------------------------------------------------------
function Speller:buildButtonTable_ABC(UseSpace)

    -- build table self.tttButtons
    local xWidget, yWidget = GetPosition(self.cself)
    local nSpacingX = 74
    local nSpacingY = 90
    local nBorder   = 2
    --local sBitmap   = "1656565652656565656536565654"
    local sBitmap   = "1666666661666666666636666664"
    
    if UseSpace == false then 
      --sBitmap   = "165656565265656565653656565656" 
      sBitmap   = "166666666166666666663666666666" 
    end
    
    local nLeftOffset = 32
    local nTopOffset = yWidget --91
    local nCurrentDisplayWidth = GetDisplayWidth(self.cself)
    
    if nCurrentDisplayWidth == 480 then
      nSpacingX = 44
      nSpacingY = 51
      nBorder   = 2
      nLeftOffset = 19
      --nTopOffset = 55
    elseif nCurrentDisplayWidth == 400 then
      nSpacingX = 37
      nSpacingY = 45
      nBorder   = 2
      nLeftOffset = 16
      --nTopOffset = 46
    end
    
    local ttButtons = {}
    for i = 1, #sBitmap do
        local nBmpType = tonumber(string.sub(sBitmap, i, i))
        local nBmpWidth, nBmpHeight = 0
        if nBmpType then nBmpWidth, nBmpHeight = GetBitmapSize(self.cself, self.ttBitmaps[1][nBmpType]) end
        ttButtons[i] = {
            x          = ((i - 1) % 10) * nSpacingX + nLeftOffset - xWidget,
            y          = ((i - 1) / 10) * nSpacingY + nTopOffset - yWidget,
            nWidth     = nSpacingX - nBorder,
            nHeight    = nSpacingY - nBorder,
            nType      = nBmpType,
            nTouchWidth = nBmpWidth,
            nTouchHeight = nBmpHeight
        }
    end
    
    if UseSpace == true then
      self.tttButtons[Speller.ABC_LAYOUT] = ttButtons
      self.tNumButtons[Speller.ABC_LAYOUT] = #self.tttButtons[Speller.ABC_LAYOUT]
      self.tttButtons[Speller.ABC_LAYOUT][self.tNumButtons[Speller.ABC_LAYOUT]].nWidth = nSpacingX * 3 - nBorder
    else
      self.tttButtons[Speller.ABC_LAYOUT_EXT] = ttButtons
      self.tNumButtons[Speller.ABC_LAYOUT_EXT] = #self.tttButtons[Speller.ABC_LAYOUT_EXT]
    end
end
------------------------------------------------------------------------------------
-- function         : buildButtonTable_NumBlock
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : Build a table for the NumBlock layout having 12 buttons and adds it
--                    to self.tttButtons. Dynamic data
--                    (depending on API calls) will not be correctly set.
--                    Use updateEnabled() for dynamic data.
------------------------------------------------------------------------------------
function Speller:buildButtonTable_NumBlock()

    -- build table self.tttButtons
    local xWidget, yWidget = GetPosition(self.cself)
    local nSpacingX = 150
    local nSpacingY = 70
    local nBorder   = 0
    --local sBitmap   = "8ca7b98ca7b9"
    local sBitmap   = "7c97c97c97c9"
    local nLeftOffset = 175--290
    local nTopOffset = yWidget --91
    local nCurrentDisplayWidth = GetDisplayWidth(self.cself)
    
    if nCurrentDisplayWidth == 480 then
      nSpacingX = 90
      nSpacingY = 40
      nLeftOffset = 105
      --nTopOffset = 60
    elseif nCurrentDisplayWidth == 400 then
      nSpacingX = 30
      nSpacingY = 10
      nLeftOffset = 35
      --nTopOffset = 15
    end
    
    local ttButtons = {}
    for i = 1, #sBitmap do
        local nBmpType = tonumber(string.sub(sBitmap, i, i),16)
        local nBmpWidth, nBmpHeight = 0
        if nBmpType then nBmpWidth, nBmpHeight = GetBitmapSize(self.cself, self.ttBitmaps[1][nBmpType]) end
        ttButtons[i] = {
            x          = ((i - 1) % 3) * nSpacingX + nLeftOffset - xWidget,
            y          = ((i - 1) / 3) * nSpacingY + nTopOffset - yWidget,
            nWidth     = nSpacingX - nBorder,
            nHeight    = nSpacingY - nBorder,
            nType      = nBmpType,
            nTouchWidth = nBmpWidth,
            nTouchHeight = nBmpHeight
        }
    end
    self.tttButtons[Speller.NUMBLOCK_LAYOUT] = ttButtons
    self.tNumButtons[Speller.NUMBLOCK_LAYOUT] = #self.tttButtons[Speller.NUMBLOCK_LAYOUT]
end

------------------------------------------------------------------------------------
-- function         : buildButtonTable_ABC_47Btn
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : Build a table for the ABC layout having 47 buttons and adds it
--                    to self.tttButtons. Dynamic data
--                    (depending on API calls) will not be correctly set.
--                    Use updateEnabled() for dynamic data.
------------------------------------------------------------------------------------
function Speller:buildButtonTable_ABC_47Btn()

    -- build table self.tttButtons
    local xWidget, yWidget = GetPosition(self.cself)
    local nSpacingX = 60
    local nSpacingY = 70
    local nBorder   = 0 -- border is designed in the button images
    --local sBitmap   = "16565656565265656565656565656565656536565656564"
    --local sBitmap   = "dijijijijijgejijijijijifdijijijijijgejijijijijh"
    local sBitmap   = "djjjjjjjjjjfdjjjjjjjjjjfdjjjjjjjjjjfdjjjjjjjjjh"
    
    
    local nLeftOffset = 40
    local nTopOffset = yWidget--91
    local nCurrentDisplayWidth = GetDisplayWidth(self.cself)
    
    if nCurrentDisplayWidth == 480 then
      nSpacingX = 36
      nSpacingY = 40
      nBorder   = 0 -- border is designed in the button images
      nLeftOffset = 24
    elseif nCurrentDisplayWidth == 400 then
      nSpacingX = 12
      nSpacingY = 10
      nBorder   = 0 -- border is designed in the button images
      nLeftOffset = 8
    end
    
    local ttButtons = {}
    for i = 1, #sBitmap do
        local nBmpType = tonumber(string.sub(sBitmap, i, i),36)
        local nBmpWidth, nBmpHeight = 0
        if nBmpType then nBmpWidth, nBmpHeight = GetBitmapSize(self.cself, self.ttBitmaps[1][nBmpType]) end
        ttButtons[i] = {
            x          = ((i - 1) % 12) * nSpacingX + nLeftOffset - xWidget,
            y          = ((i - 1) / 12) * nSpacingY + nTopOffset - yWidget,
            nWidth     = nSpacingX - nBorder,
            nHeight    = nSpacingY - nBorder,
            nType      = nBmpType,
            nTouchWidth = nBmpWidth,
            nTouchHeight = nBmpHeight
        }
    end
    
    self.tttButtons[Speller.ABC_LAYOUT_47BTN] = ttButtons
    self.tNumButtons[Speller.ABC_LAYOUT_47BTN] = #self.tttButtons[Speller.ABC_LAYOUT_47BTN]
    self.tttButtons[Speller.ABC_LAYOUT_47BTN][self.tNumButtons[Speller.ABC_LAYOUT_47BTN]].nWidth = nSpacingX * 2 - nBorder

end

------------------------------------------------------------------------------------
-- function         : buildButtonTable_QWERTY
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : Build a table for the QWERTY layout having 36 buttons and adds it
--                    to self.tttButtons. Dynamic data
--                    (depending on API calls) will not be correctly set.
--                    Use updateEnabled() for dynamic data.
------------------------------------------------------------------------------------
function Speller:buildButtonTable_QWERTY()

    -- build table self.tttButtons
    local xWidget, yWidget = GetPosition(self.cself)
    local nSpacingX = 75
    local nSpacingY = 70
    local nBorder   = 0
    --local sBitmap   = "265656565315656565326565656531564653"
    local sBitmap   = "166666666316666666316666666631664663"
    --local sBitmap_NumBlock   = "565656565665"
    local sBitmap_NumBlock   = "666666666666"
    local nLeftOffset_10Btn = xWidget  -- used for rows with 10 buttons
    local nLeftOffset_9Btn =  xWidget + 35 -- used for rows with 9 buttons
    local nTopOffset = yWidget
    local nCurrentDisplayWidth = GetDisplayWidth(self.cself)
    
    if nCurrentDisplayWidth == 480 then
      nSpacingX = 45
      nSpacingY = 40
      nBorder   = 0
      --nLeftOffset_10Btn = 15
      nLeftOffset_9Btn = xWidget + 21
      --nTopOffset = 104
    elseif nCurrentDisplayWidth == 400 then
      nSpacingX = 15
      nSpacingY = 10
      nBorder   = 0
      --nLeftOffset_10Btn = 5
      nLeftOffset_9Btn = xWidget + 7
      --nTopOffset = 26
    end
    local ttButtons = {}
    for i = 1, #sBitmap do
        local nBmpType = tonumber(string.sub(sBitmap, i, i))
        local nBmpWidth, nBmpHeight = 0
        if nBmpType then nBmpWidth, nBmpHeight = GetBitmapSize(self.cself, self.ttBitmaps[1][nBmpType]) end
        if i <= 10 then
            ttButtons[i] = {
                x          = ((i - 1) % 10) * nSpacingX + nLeftOffset_10Btn - xWidget,
                y          = ((i - 1) / 10) * nSpacingY + nTopOffset - yWidget,
                nWidth     = nSpacingX - nBorder,
                nHeight    = nSpacingY - nBorder,
                nType      = nBmpType,
                nTouchWidth = nBmpWidth,
                nTouchHeight = nBmpHeight
               }
        elseif i >= 11 and i <20 then
            ttButtons[i] = {
                x          = ((i - 1) % 10) * nSpacingX + nLeftOffset_9Btn - xWidget,
                y          = ((i - 1) / 10) * nSpacingY + nTopOffset - yWidget,
                nWidth     = nSpacingX - nBorder,
                nHeight    = nSpacingY - nBorder,
                nType      = nBmpType,
                nTouchWidth = nBmpWidth,
                nTouchHeight = nBmpHeight
                }
        elseif i >= 20 and i <30 then
            ttButtons[i] = {
                x          = (i % 10) * nSpacingX + nLeftOffset_10Btn - xWidget,
                y          = (i / 10) * nSpacingY + nTopOffset - yWidget,
                nWidth     = nSpacingX - nBorder,
                nHeight    = nSpacingY - nBorder,
                nType      = nBmpType,
                nTouchWidth = nBmpWidth,
                nTouchHeight = nBmpHeight
               }
        -- this is the space bar
        elseif i == 33 then
            ttButtons[i] = {
                x          = (i % 10) * nSpacingX + nLeftOffset_9Btn - xWidget,
                y          = (i / 10) * nSpacingY + nTopOffset - yWidget,
                nWidth     = nSpacingX * 3 - nBorder,
                nHeight    = nSpacingY - nBorder,
                nType      = nBmpType,
                nTouchWidth = nBmpWidth,
                nTouchHeight = nBmpHeight
                }
        elseif i >= 34 then
            ttButtons[i] = {
                x          = ((i+2) % 10) * nSpacingX + nLeftOffset_9Btn - xWidget,
                y          = (i / 10) * nSpacingY + nTopOffset - yWidget,
                nWidth     = nSpacingX - nBorder,
                nHeight    = nSpacingY - nBorder,
                nType      = nBmpType,
                nTouchWidth = nBmpWidth,
                nTouchHeight = nBmpHeight
                }
        elseif i >= 30 then
            ttButtons[i] = {
                x          = (i % 10) * nSpacingX + nLeftOffset_9Btn - xWidget,
                y          = (i / 10) * nSpacingY + nTopOffset - yWidget,
                nWidth     = nSpacingX - nBorder,
                nHeight    = nSpacingY - nBorder,
                nType      = nBmpType,
                nTouchWidth = nBmpWidth,
                nTouchHeight = nBmpHeight
                }
        end
    end
    
    self.tttButtons[Speller.QWERTY_LAYOUT] = ttButtons
    
    self.tNumButtons[Speller.QWERTY_LAYOUT] = #self.tttButtons[Speller.QWERTY_LAYOUT]
end
------------------------------------------------------------------------------------
-- function         : updateEnabled
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : Update the enabled flag or all buttons, and dynamically
--                    assign additional letters to buttons with bFill flag set.
------------------------------------------------------------------------------------
function Speller:updateEnabled()

    local sCharacters = GetProperty(self.cself, "GetLetterFunction", GETPROPERTY_DATATYPE_STRING) or ""
    local bInvert = (GetProperty(self.cself, "InvertGetLetterFunction") or 0) ~= 0
    self.bEnabled = (GetProperty(self.cself, "Enabled") or 1) ~= 0
    local nLayout = 0

    --[[ test code
    sCharacters = "ABCDabcd1234"
    bInvert = false
    self.bEnabled = true
    --]]

    local tCharacters = { UTF8Decode(sCharacters) }
    local tCharacterUsed = { }
    for _, nCharacter in ipairs(tCharacters) do
        tCharacterUsed[nCharacter] = false
    end

    -- set all defined buttons of the layout, and collect fill buttons
    local ttFillButtonDefs = { }
    local bUseFill = self.tttLayouts.bUseFill and not bInvert and self.bEnabled
    for _, ttLayout in ipairs(self.tttLayouts) do
        for _, tButtonDef in ipairs(ttLayout) do
            if tButtonDef.bFill then
                tButtonDef.sText = ""
                tButtonDef.bEnabled = false
                tButtonDef.nCharacter = 0
                if bUseFill then
                    ttFillButtonDefs[#ttFillButtonDefs + 1] = tButtonDef
                end
            else
                local nCharacter = tButtonDef.nCharacter
                if nCharacter ~= 0 then
                    if tCharacterUsed[nCharacter] ~= nil then
                        tCharacterUsed[nCharacter] = true
                        tButtonDef.bEnabled = not bInvert and self.bEnabled
                    else
                        tButtonDef.bEnabled = bInvert and self.bEnabled
                    end
                end
            end
            if tButtonDef.nCharacter == self.nSpace then
                tButtonDef.bEnabled = tButtonDef.bEnabled and self.bSpaceBtnEnabled
            end
        end
    end

    -- dynamically allocate fill buttons
    if bUseFill then
        local nFill = 0
        for _, nCharacter in ipairs(tCharacters) do
            if not tCharacterUsed[nCharacter] then
                nFill = nFill + 1
                local tButtonDef = ttFillButtonDefs[nFill]
                if not tButtonDef then
                    break
                end
                tButtonDef.nCharacter = nCharacter
                tButtonDef.sText = UTF8Encode(nCharacter)
                tButtonDef.bEnabled = true
                tCharacterUsed[nCharacter] = true
            end
        end
    end

    -- determine bVisible state of all layouts
    for _, ttLayout in ipairs(self.tttLayouts) do

        ttLayout.bVisible = false
        local bIsCharAvail = false
        local bIsSpaceAvail = false
        for _, tButtonDef in ipairs(ttLayout) do
            if tButtonDef.nCharacter ~= 0 and tButtonDef.nCharacter ~= self.nSpace then
                bIsCharAvail = true -- Character is available
                if tButtonDef.bEnabled ~= false then -- Character is enabled
                   ttLayout.bVisible = true
                   break
                end
            elseif tButtonDef.nCharacter == self.nSpace and tButtonDef.bEnabled ~= false then
                bIsSpaceAvail = true -- Space bar is available and enabled
            end
        end
        -- To handle situation where characters are available but not enabled, with space bar enabled
        -- Change for issue NIKAI-1796, Consider layout only with space character just for the first layout,
        -- otherwise the spacebar is accomodated in the previous layouts.
        if ttLayout.bVisible ~= true and nLayout == 0 then
            if bIsCharAvail == true and bIsSpaceAvail == true then ttLayout.bVisible = true end
        end
        if ttLayout.bVisible ==true then nLayout = nLayout + 1 end

    end

    if self.pToggleButton ~= nil then
        local bBtnEnabled = (GetProperty(self.pToggleButton, "Enabled") or 1) ~= 0
        if nLayout > 1 and bBtnEnabled == false then
            SetProperty(self.pToggleButton, "Enabled", tBoolean[true])
            WidgetInvalidate(self.pToggleButton)
        elseif nLayout <= 1 and bBtnEnabled == true then
            SetProperty(self.pToggleButton, "Enabled", tBoolean[false])
            WidgetInvalidate(self.pToggleButton)
        end
    end

    if not self.tttLayouts[self.nLayout].bVisible and self.bEnabled then
        self:switchLayout()
    end
    
    if self.bGotFocus then

        if not self:repositionCursor(self.nCursor) then
            -- hand over focus to button container (first enabled button)
            self:focusLost()
            SendMessage(self.pButtonContainer, GUI_EVENT_WIDGET_SETFOCUS, self.cidself, NumberToCid(1))
        end

    end

end


------------------------------------------------------------------------------------
-- function         : switchLayout
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : Switch the layout between alphabetic and numeric characters.
--                    Switches to the next layout that has buttons with a text, or
--                    to the first layout if there are no layouts with text on buttons.
------------------------------------------------------------------------------------
function Speller:switchLayout()

    -- determine next layout
    local nLayout = self.nLayout
    local nNumLayouts = #self.tttLayouts
    for i = 1, nNumLayouts do
        if nLayout >= nNumLayouts then
            nLayout = 1
        else
            nLayout = nLayout + 1
        end
        if self.tttLayouts[nLayout].bVisible then
            break
        end
    end
    if self.tttLayouts[nLayout].bVisible then
        self.nLayout = nLayout
    else
        self.nLayout = math.limit(self.nLayout, 1, nNumLayouts)
    end
    
    
    if #self.tttLayouts[self.nLayout] == #self.tttButtons[Speller.ABC_LAYOUT] then
       self.nCurrentLayoutType = Speller.ABC_LAYOUT
    elseif #self.tttLayouts[self.nLayout] == #self.tttButtons[Speller.ABC_LAYOUT_EXT] then
       self.nCurrentLayoutType = Speller.ABC_LAYOUT_EXT
    elseif #self.tttLayouts[self.nLayout] == #self.tttButtons[Speller.ABC_LAYOUT_47BTN] then
       self.nCurrentLayoutType = Speller.ABC_LAYOUT_47BTN
    elseif #self.tttLayouts[self.nLayout] == #self.tttButtons[Speller.QWERTY_LAYOUT] then
       self.nCurrentLayoutType = Speller.QWERTY_LAYOUT
    elseif #self.tttLayouts[self.nLayout] == #self.tttButtons[Speller.NUMBLOCK_LAYOUT] then
       self.nCurrentLayoutType = Speller.NUMBLOCK_LAYOUT
    else
       TTFis_Trace(self.cself,"Unknown Layout with "..tostring(#self.tttLayouts[self.nLayout]).." characters")
    end
    
    self.bPressed = false

    if self.bGotFocus then

        --[[ old behavior: try to keep focus, hand over to button container if it does not work out
        if not self:repositionCursor() then
            -- hand over focus to button container (first enabled button)
            self:focusLost()
            SendMessage(self.pButtonContainer, GUI_EVENT_WIDGET_SETFOCUS, self.cidself, NumberToCid(1))
        end
        --]]

        -- new behavior: set focus to button container (invisible focus)
        self.nCursor = nil
        self:focusLost()
        SendMessage(self.pButtonContainer, GUI_EVENT_WIDGET_SETFOCUS, self.cidself, NumberToCid(3))
    end

end


------------------------------------------------------------------------------------
-- function         : preDraw
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : The preDraw function is called during a redraw before the
--                    child widgets' draw function will be called. The parameters
--                    specify the x,y coordinates of the widget.
------------------------------------------------------------------------------------
function Speller:preDraw(x, y)

    for b, tButton in ipairs(self.tttButtons[self.nCurrentLayoutType]) do

        -- determine state of the button
        local tButtonDef = self.tttLayouts[self.nLayout][b]
        local nState = 4 -- default: disabled state
        if tButtonDef.bEnabled then
            if self.bPressed and b == self.nPressed then
                nState = 3 -- pressed state
            elseif b == self.nCursor then
                nState = 2 -- focused state
            else
                nState = 1 -- enabled state
            end
        end

        -- draw button
        local xButton = x + tButton.x
        local yButton = y + tButton.y
        local cidBitmap = self.ttBitmaps[nState][tButton.nType]
        if cidBitmap then
            DrawBitmap(self.cself, cidBitmap, xButton, yButton, xButton + tButton.nWidth - 1, yButton + tButton.nHeight - 1)
        else
            TTFis_Trace(self.cself,"Speller:predraw No Bitmap found for State "..tostring(nState).." and Type "..tostring(tButton.nType).." in layout "..tostring(self.nLayout))
        end
        local nButImgWidth, nButImgHeight = GetBitmapSize(self.cself, cidBitmap)
        -- draw text on button
        if tButtonDef.sText ~= "" then
            local nTextWidth = GetTextWidth(self.cself, tButtonDef.sText, self.nFontId)
            local xText = xButton + (nButImgWidth - nTextWidth)/2
            local yText = yButton + (nButImgHeight - self.nFontHeight)/2
            local nColor = self.ttColors[nState][tButton.nType] or 0
            --TTFis_Trace(self.cself,"Speller: y/yB/yT/Basel: "..y.." / "..yButton.." / "..yText.." / "..self.nBaseline)
            if self.nBaseline == 0 then
               DrawText(self.cself, xText, yText, nil, nil, tButtonDef.sText, self.nFontId, nColor, 0)
            else
               DrawText(self.cself, xText, yText, nil, nil, tButtonDef.sText, self.nFontId, nColor, yButton + self.nBaseline)
            end
        end
    end

end


------------------------------------------------------------------------------------
-- function         : onMessage
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : The onMessage function is called when a message has been sent
--                    to the widget. The two parameters are optional and can be nil.
--
--  return          : 0 - message not consumed
--                    1 - message consumed
------------------------------------------------------------------------------------
-- History:
--  Grohs             27.06.2008 - #3624: Return result of message consumption
------------------------------------------------------------------------------------
function Speller:onMessage(eventid, param1, param2)

    local nMsgConsumed = 0

    --print("Speller: got message " .. eventid .. ", param1 " .. tostring(param1) .. ", param2 " .. tostring(param2))

    local nMove

    if eventid == GUI_EVENT_HK_MAIN_NEG then

        nMove = -1
        nMsgConsumed = 1

    elseif eventid == GUI_EVENT_HK_MAIN_POS then

        nMove = 1
        nMsgConsumed = 1

    elseif eventid == GUI_EVENT_WIDGET_SETFOCUS then

        if type(param1) == "userdata" then

            self.pButtonContainer = param1

            --print("Speller got SetFocus(" .. tostring(param2) .. ") - bGotFocus " .. tostring(self.bGotFocus))
            if not self.bGotFocus then

                -- try to reposition cursor on first button (if param2==1) or last button (if param2==2)
                if self:repositionCursor((param2 - 1) * self.tNumButtons[self.nCurrentLayoutType]) then
                    self:focusGained()
                    local x1 = self.x + self.tttButtons[self.nCurrentLayoutType][self.nCursor].x
                    local y1 = self.y + self.tttButtons[self.nCurrentLayoutType][self.nCursor].y
                    local x2 = self.x + self.tttButtons[self.nCurrentLayoutType][self.nCursor].x + self.tttButtons[self.nCurrentLayoutType][self.nCursor].nWidth - 1
                    local y2 = self.y + self.tttButtons[self.nCurrentLayoutType][self.nCursor].y + self.tttButtons[self.nCurrentLayoutType][self.nCursor].nHeight - 1
                    InvalidateRect(self.cself, x1,y1,x2,y2)
                    --WidgetInvalidate(self.cself)
                else
                    -- positioning of cursor failed -> tell button container about it
                    SendMessage(self.pButtonContainer, GUI_EVENT_WIDGET_SETFOCUS, self.cidself, NumberToCid(2))
                end

            end

        end

    elseif eventid == self.nToggleLayoutEvent then

        self:switchLayout()
        WidgetInvalidate(self.cself)
        nMsgConsumed = 1

    elseif eventid == GUI_EVENT_HK_MAIN_PRESS   or
           eventid == GUI_EVENT_HK_MAIN_LONG    or
           eventid == GUI_EVENT_HK_MAIN_REPEAT  or
           eventid == GUI_EVENT_HK_MAIN_SHORT   or
           eventid == GUI_EVENT_HK_MAIN_RELEASE then

        -- remember pressed button
        if eventid == GUI_EVENT_HK_MAIN_PRESS then
            self.nPressed = self.nCursor
        end

        self:press(param1)
        nMsgConsumed = 1

    elseif eventid == GUI_EVENT_WIDGET_VIEW_CHANGE then

        -- do not consume the view-change message as there might be more
        -- widgets that are interested!
        if self.bPressed then
            self.bPressed = false
            WidgetInvalidate(self.cself)
        end

    elseif eventid == GUI_EVENT_WIDGET_ENTRYFIELD_EMPTY then
    
        --disable space button
        if param2 == 1 then
            self.bSpaceBtnEnabled = false
        else
            self.bSpaceBtnEnabled = true
        end

        self:updateEnabled()

        WidgetInvalidate(self.cself)
        
        nMsgConsumed = 1
    end

    if nMove and self.nCursor then

        self:moveCursor(nMove)

    end

    return nMsgConsumed

end


------------------------------------------------------------------------------------
-- function         : moveCursor
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : Assumes that there is a valid cursor position to start with.
--                    Returns true if the cursor could be repositioned.
------------------------------------------------------------------------------------
function Speller:moveCursor(nMove)

    local ttLayout = self.tttLayouts[self.nLayout]
    local ttButtons = self.tttButtons[self.nCurrentLayoutType]
    local nCursor = self.nCursor + nMove

    TTFis_Trace(self.cself, "moveCursor")
    while (nCursor > 0) and (nCursor <= #ttLayout) do

        if ttLayout[nCursor].bEnabled then
            if self.nCursor <= #ttLayout then
               -- invalidate the previously focused button
               TTFis_Trace(self.cself, "invalidate old focused button " .. tostring(self.nCursor))
               local x = ttButtons[self.nCursor].x
               local y = ttButtons[self.nCursor].y
               local width = ttButtons[self.nCursor].nWidth
               local height = ttButtons[self.nCursor].nHeight
               TTFis_Trace(self.cself, "button rect " .. tostring(x) .. ", " .. tostring(y) .. ", " .. tostring(width) .. ", " .. tostring(height) )
               InvalidateRect(self.cself, self.x + x, self.y + y, self.x + x + width-1, self.y + y + height-1)
               --WidgetInvalidate(ttLayout[self.nCursor])
            end
            self.nCursor = nCursor
            -- invalidate the new focused button
            local x = ttButtons[self.nCursor].x
            local y = ttButtons[self.nCursor].y
            local width = ttButtons[self.nCursor].nWidth
            local height = ttButtons[self.nCursor].nHeight
            TTFis_Trace(self.cself, "button rect " .. tostring(x) .. ", " .. tostring(y) .. ", " .. tostring(width) .. ", " .. tostring(height) )
            InvalidateRect(self.cself, self.x + x, self.y + y, self.x + x + width-1, self.y + y + height-1)
            --WidgetInvalidate(ttLayout[self.nCursor])
            TTFis_Trace(self.cself, "invalidate new focused button " .. tostring(self.nCursor))
            return true
        end

        nCursor = nCursor + nMove

    end
    
    TTFis_Trace(self.cself, "no button found")
    
    if nCursor <= 0 then
        -- move cursor to button container
        self:focusLost()
        -- invalidate the previously focused button
        if self.nCursor ~= nil and self.nCursor <= #ttLayout then
           local x = ttButtons[self.nCursor].x
           local y = ttButtons[self.nCursor].y
           local width = ttButtons[self.nCursor].nWidth
           local height = ttButtons[self.nCursor].nHeight
           TTFis_Trace(self.cself, "button rect " .. tostring(x) .. ", " .. tostring(y) .. ", " .. tostring(width) .. ", " .. tostring(height) )
           InvalidateRect(self.cself, self.x + x, self.y + y, self.x + x + width-1, self.y + y + height-1)
           --WidgetInvalidate(ttLayout[self.nCursor])
           TTFis_Trace(self.cself, "invalidate old focused button " .. tostring(self.nCursor))
        end
        self.nCursor = nil
        SendMessage(self.pButtonContainer, GUI_EVENT_WIDGET_SETFOCUS, self.cidself, NumberToCid(1))
        return true
    end

end


------------------------------------------------------------------------------------
-- function         : repositionCursor
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : Repositioning of the cursor, as close as possible to the
--                    specified default position (default 1).
------------------------------------------------------------------------------------
function Speller:repositionCursor(nDefaultPos)

    local ttLayout = self.tttLayouts[self.nLayout]
    self.nCursor = nil
    nDefaultPos = nDefaultPos or 1

    -- check for nearest enabled button
    for i = 1, self.tNumButtons[self.nCurrentLayoutType] do

        local nIndex = nDefaultPos - i + 1
        if nIndex > 0 and nIndex <= self.tNumButtons[self.nCurrentLayoutType] and (ttLayout[nIndex].bEnabled or not self.bEnabled) then
            self.nCursor = nIndex
            return true
        end

        nIndex = nDefaultPos + i
        if nIndex > 0 and nIndex <= self.tNumButtons[self.nCurrentLayoutType] and ttLayout[nIndex].bEnabled then
            self.nCursor = nIndex
            return true
        end

    end

end


------------------------------------------------------------------------------------
-- function         : focusGained
------------------------------------------------------------------------------------
-- Copyright        : 2008 Robert Bosch GmbH, Hildesheim
-- Author           : Doetsch
------------------------------------------------------------------------------------
-- Description      : Register to DDS events.
------------------------------------------------------------------------------------
function Speller:focusGained()

    if not self.bGotFocus then
        self.bGotFocus = true
        RegisterEvent(self.cself, GUI_EVENT_HK_MAIN_NEG,    10)
        RegisterEvent(self.cself, GUI_EVENT_HK_MAIN_POS,    10)
        RegisterEvent(self.cself, GUI_EVENT_HK_MAIN_PRESS,  10)
        RegisterEvent(self.cself, GUI_EVENT_HK_MAIN_LONG,   10)
        RegisterEvent(self.cself, GUI_EVENT_HK_MAIN_REPEAT, 10)
        RegisterEvent(self.cself, GUI_EVENT_HK_MAIN_SHORT,  10)
        RegisterEvent(self.cself, GUI_EVENT_HK_MAIN_RELEASE,10)
    end

end


------------------------------------------------------------------------------------
-- function         : focusLost
------------------------------------------------------------------------------------
-- Copyright        : 2008 Robert Bosch GmbH, Hildesheim
-- Author           : Doetsch
------------------------------------------------------------------------------------
-- Description      : Deregister from DDS events.
------------------------------------------------------------------------------------
-- History          : Remove unused 3rd parameter priority
------------------------------------------------------------------------------------
function Speller:focusLost()

    if self.bGotFocus then
        self.bGotFocus = false
        DeregisterEvent(self.cself, GUI_EVENT_HK_MAIN_NEG)
        DeregisterEvent(self.cself, GUI_EVENT_HK_MAIN_POS)
        DeregisterEvent(self.cself, GUI_EVENT_HK_MAIN_PRESS)
        DeregisterEvent(self.cself, GUI_EVENT_HK_MAIN_LONG)
        DeregisterEvent(self.cself, GUI_EVENT_HK_MAIN_REPEAT)
        DeregisterEvent(self.cself, GUI_EVENT_HK_MAIN_SHORT)
        DeregisterEvent(self.cself, GUI_EVENT_HK_MAIN_RELEASE)
    end

end


------------------------------------------------------------------------------------
-- function         : isInTouchArea
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : The isInTouchArea function is called to ask a widget if it
--                    is responsible for a touch click. The function should return
--                    0 (not in touch area) or 1 (in touch area).
------------------------------------------------------------------------------------
function Speller:isInTouchArea(widgetX, widgetY, clickX, clickY)

    -- check all enabled buttons
    local ttLayout = self.tttLayouts[self.nLayout]
    for b, tButton in ipairs(self.tttButtons[self.nCurrentLayoutType]) do
        if ttLayout[b].bEnabled then
            local buttonX = widgetX + tButton.x
            local buttonY = widgetY + tButton.y
            if clickX >= buttonX and clickX < buttonX + tButton.nTouchWidth and
               clickY >= buttonY and clickY < buttonY + tButton.nTouchHeight then
                self.nPressed = b
                return 1
            end
        end
    end

    return 0

end


------------------------------------------------------------------------------------
-- function         : onTouch
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : The onTouch function is called to notify a touch event.
------------------------------------------------------------------------------------
function Speller:onTouch(duration, x, y)

    --print("onTouch " .. duration .. ", (" .. x .. "," .. y .. ")")
    if duration == DURATION_PRESS then

        -- create click sound
        CreateClickSound()

    end

    self:press(duration)

end


------------------------------------------------------------------------------------
-- function         : press
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : Called when a press event (all durations, i.e. including
--                    repeat, release, etc.) has to be handled, no matter if it
--                    was triggered by DDS or by touch. The pressed button is
--                    the cursor button as defined by self.nPressed and self.nLayout.
------------------------------------------------------------------------------------
function Speller:press(duration)

    if duration == DURATION_PRESS then

        self.bPressed = true
        local nCharacter = (self.nPressed and self.tttLayouts[self.nLayout][self.nPressed].nCharacter) or 0
        SetProperty(self.cself, "PressedCharacter", math._and(nCharacter, CID_DATA_MASK) + CID_OFFSET_CHARACTER, true, true)
        PerformPropertyAction(self.cself, "OUT_EVENT_pressed")
        WidgetInvalidate(self.cself)
    elseif duration == DURATION_LONGPRESS then

        PerformPropertyAction(self.cself, "OUT_EVENT_longpressed")

    elseif duration == DURATION_REPEAT then

        PerformPropertyAction(self.cself, "OUT_EVENT_repeat")

    elseif duration == DURATION_SHORTRELEASE then

        PerformPropertyAction(self.cself, "OUT_EVENT_shortreleased")

    elseif duration == DURATION_RELEASE then

        self.bPressed = false
        PerformPropertyAction(self.cself, "OUT_EVENT_released")
        WidgetInvalidate(self.cself)

    end

end


------------------------------------------------------------------------------------
-- function         : onDataUpdate
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : This function is called when a data update event is triggered
--                    and the widget has registered to the event using the
--                    RegisterDataUpdate() function.
------------------------------------------------------------------------------------
function Speller:onDataUpdate(nUserId)

    self:updateEnabled()
    WidgetInvalidate(self.cself)

end

------------------------------------------------------------------------------------
-- Function         : getAbsoluteChildIndex()
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
-- Description      : called when a child should be addressed by its index.
--
--  input params    : nIndex - index of the requested child 
--  return          : 0 and the coordinates if nIndex addresses a valid child else negative errorcode
--  impact          : none
--  constraints     : none
------------------------------------------------------------------------------------
------------------------------------------------------------------------------------
function Speller:getAbsoluteChildIndex(nIndex)
    
    local x = -1
    local y = -1
    local nErrorCode = 0
    
    if nIndex ~= 0 and nIndex <= self.tNumButtons[self.nLayout] then
        x = self.tttButtons[self.nLayout][nIndex].x
        y = self.tttButtons[self.nLayout][nIndex].y
    else
        nErrorCode = -1
    end
    
    return nErrorCode, x, y
end
