--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_CreateRoom_C
local M = UnLua.Class()

local WBP_MainMenuClass = UE.UClass.Load("/OnlineSession/UI/WBP_MainMenu.WBP_MainMenu_C")

--function M:Initialize(Initializer)
--end

--function M:PreConstruct(IsDesignTime)
--end

function M:Construct()
    self:ButtonBinding()
end

--function M:Tick(MyGeometry, InDeltaTime)
--end


--- 按键绑定
function M:ButtonBinding()
    self.ServerNameTextBox.OnTextChanged:Add(self, self.OnTextChanged_ServerName)
    self.AmountOfSlotsComboBox.OnSelectionChanged:Add(self, self.OnSelectionChanged_AmountOfSlots)
    self.CreateServerButton.OnClicked:Add(self, self.OnClicked_CreateServer)
    self.BackButton.OnClicked:Add(self, self.OnClicked_Back)
end

--- 在文本更改时服务器名字
---@param Text string
function M:OnTextChanged_ServerName(Text)
    self.ServerName = Text
end

---
---@param SelectedItem string
---@param SelectionType number
function M:OnSelectionChanged_AmountOfSlots(SelectedItem, SelectionType)
    self.AmountOfSlots = tonumber(SelectedItem)
end

--- 创建房间按下
function M:OnClicked_CreateServer()

end

--- 返回按下
function M:OnClicked_Back()
    local MainMenu = UE.UWidgetBlueprintLibrary.Create(self, WBP_MainMenuClass)
    MainMenu:AddToViewport()
    self.RemoveFromParent(self)
end

return M
