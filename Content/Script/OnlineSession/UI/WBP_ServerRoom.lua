--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type WBP_ServerRoom_C
local M = UnLua.Class()

--function M:Initialize(Initializer)
--end

--function M:PreConstruct(IsDesignTime)
--end

function M:Construct()
    self:ButtonBinding()
    self.NameRoom:SetText(self:GetServerRoomName())
    self.NumberRoom:SetText(self:GetNumberRoom())
    self.RoomDelay:SetText(self:GetRoomDelay())
end

--function M:Tick(MyGeometry, InDeltaTime)
--end

--- 按键绑定
function M:ButtonBinding()
    self.JoinRoomButton.OnClicked:Add(self, self.OnClicked_JoinRoom)
end

--- 加入游戏
function M:OnClicked_JoinRoom()
    self:DestroySession()
    local JoinSessionCallback = UE.UJoinSessionCallbackProxy.JoinSession(self, self:GetOwningPlayer(), self.SearchResult)
    JoinSessionCallback.OnSuccess:Add(self, self.JoinGameSuccess)
    JoinSessionCallback:Activate()
end

--- 成功加入游戏
function M:JoinGameSuccess()
    print("成功加入游戏")
    self:RemoveFromParent()
    UE.UWidgetBlueprintLibrary.SetInputMode_GameOnly(self:GetOwningPlayer(), false)
end

--- 删除会话
function M:DestroySession()
    local SessionCallback = UE.UDestroySessionCallbackProxy.DestroySession(self, self:GetOwningPlayer())
    --SessionCallback.OnSuccess:Add(self, self.fun)
    --SessionCallback.OnFailure:Add(self, self.fun)
end

--- 获取房间人数
function M:GetNumberRoom()
    return UE.UFindSessionsCallbackProxy.GetCurrentPlayers(self.SearchResult) .. "/" .. UE.UFindSessionsCallbackProxy.GetMaxPlayers(self.SearchResult)
end

--- 获取房间延迟
function M:GetRoomDelay()
    return UE.UFindSessionsCallbackProxy.GetPingInMs(self.SearchResult) .. "ms"
end

return M
