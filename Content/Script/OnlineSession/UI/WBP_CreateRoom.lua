--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_CreateRoom_C
local M = UnLua.Class()

local Screen = require("Screen")

local WBP_MainMenuClass = UE.UClass.Load("/OnlineSession/UI/WBP_MainMenu.WBP_MainMenu_C")

--function M:Initialize(Initializer)
--end

--function M:PreConstruct(IsDesignTime)
--end

function M:Construct()
    self:ButtonBinding()
    self:SetSteamPlayerName()
    self:SetDefaultValue()
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

--- 在文本更改时玩家数量
---@param SelectedItem string
---@param SelectionType number
function M:OnSelectionChanged_AmountOfSlots(SelectedItem, SelectionType)
    self.AmountOfSlots = tonumber(SelectedItem)
end

--- 创建房间按下
function M:OnClicked_CreateServer()
    self:SetExtraSettings()
    UE.UDestroySessionCallbackProxy:DestroySession(self:GetOwningPlayer())
    local CreateSessionCallback = UE.UCreateSessionCallbackProxyAdvanced:CreateAdvancedSession(
            self.ExtraSettings,
            self:GetOwningPlayer(),
            self.AmountOfSlots
    )

    CreateSessionCallback.OnSuccess:Add(self, self.CreateServerSuccess)
    CreateSessionCallback.OnFailure:Add(self, self.CreateServerFailure)
    CreateSessionCallback:Activate()

end

--- 房间创建成功
function M:CreateServerSuccess()
    UE.UGameplayStatics.OpenLevel(self, "/Game/Maps/OnlineMap", true, "Listen")
    self:RemoveFromParent()
    UE.UWidgetBlueprintLibrary.SetInputMode_GameOnly(self:GetOwningPlayer(), false)
    Screen.Print("房间场景创建成功", UE.FLinearColor(0, 1, 0, 1))
end

--- 房间创建失败
function M:CreateServerFailure()

end

--- 返回按下
function M:OnClicked_Back()
    Screen.Print("返回按钮按下")
    local MainMenu = UE.UWidgetBlueprintLibrary.Create(self, WBP_MainMenuClass)
    MainMenu:AddToViewport()
    self:RemoveFromParent()
end

return M
