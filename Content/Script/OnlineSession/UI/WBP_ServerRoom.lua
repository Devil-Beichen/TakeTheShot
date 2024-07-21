--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_ServerRoom_C
local M = UnLua.Class()
local Screen = require("Screen")

--function M:Initialize(Initializer)
--end

--function M:PreConstruct(IsDesignTime)
--end

function M:Construct()
    self:SetRoomName()
    self:SetNumberRoom()
    self:SetRoomDelay()
    self:ButtonBinding()
end

--function M:Tick(MyGeometry, InDeltaTime)
--end

--- 按键绑定
function M:ButtonBinding()
    self.JoinRoomButton.OnClicked:Add(self, self.OnClicked_JoinRoom)
end

--- 加入游戏
function M:OnClicked_JoinRoom()
    Screen.Print("尝试加入游戏")
    self:DestroySession()
    local JoinSessionCallback = UE.UJoinSessionCallbackProxy.JoinSession(self, self:GetOwningPlayer(), self.SearchResult)
    JoinSessionCallback.OnSuccess:Add(self, self.JoinGameSuccess)
    JoinSessionCallback.OnFailure:Add(self, self.JoinGameFailure)
    JoinSessionCallback:Activate()
end

--- 成功加入游戏
function M:JoinGameSuccess()
    print("成功加入游戏")
    self:RemoveFromParent()
    UE.UWidgetBlueprintLibrary.SetInputMode_GameOnly(self:GetOwningPlayer(), false)
end

--- 加入游戏失败
function M:JoinGameFailure()
end

--- 删除会话
function M:DestroySession()
    UE.UDestroySessionCallbackProxy.DestroySession(self, self:GetOwningPlayer())
    --SessionCallback.OnSuccess:Add(self, self.fun)
    --SessionCallback.OnFailure:Add(self, self.fun)
end

return M
