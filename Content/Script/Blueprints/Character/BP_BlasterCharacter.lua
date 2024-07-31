--
-- DESCRIPTION
--
-- @COMPANY **
-- @AUTHOR **
-- @DATE ${date} ${time}
--

---@type BP_BlasterCharacter_C
local M = UnLua.Class()

local Screen = require("Screen")

--- 增强输入系统
local EnhancedInput = UE.UEnhancedInputLocalPlayerSubsystem

-- 增强输入绑定操作
local BindAction = UnLua.EnhancedInput.BindAction

--[[ 按键的状态
  Triggered              发生在Tick后持续
  Started                发生按下
  Ongoing                长时间按下
  Canceled               取消按下
  Completed              按键完成]]

--[[--- 移动动作的地址
local IA_Move = "/Game/Input/Actions/IA_Move.IA_Move"
--- 绑定移动操作输入持续按下响应
BindAction(M, IA_Move, "Triggered", function(self, ActionValue)
    self:Move_Triggered(ActionValue)
end)

--- 跳跃动作的地址
local IA_Jump = "/Game/Input/Actions/IA_Jump.IA_Jump"
--- 绑定跳跃操作输入按下响应
BindAction(M, IA_Jump, "Started", function(self, ActionValue)
    self:Jump_Started(ActionValue)
end)
--- 绑定跳跃操作输入按下完成响应
BindAction(M, IA_Jump, "Completed", function(self, ActionValue)
    self:Jump_Completed(ActionValue)
end)

--- 观看动作地址
local IA_Look = "/Game/Input/Actions/IA_Look.IA_Look"
--- 绑定观看操作输入持续按下响应
BindAction(M, IA_Look, "Triggered", function(self, ActionValue)
    self:Look_Triggered(ActionValue)
end)

--- 蹲下动作地址
local IA_Crouch = "/Game/Input/Actions/IA_Crouch.IA_Crouch"
--- 绑定蹲下操作输入按下响应
BindAction(M, IA_Crouch, "Started", function(self, ActionValue)
    self:Crouch_Started(ActionValue)
end)]]

-- function M:Initialize(Initializer)
-- end

-- function M:UserConstructionScript()
-- end

function M:ReceiveBeginPlay()
   -- self:AddOverheadWidget()
end

-- function M:ReceiveEndPlay()
-- end

-- function M:ReceiveTick(DeltaSeconds)
-- end

-- function M:ReceiveAnyDamage(Damage, DamageType, InstigatedBy, DamageCauser)
-- end

-- function M:ReceiveActorBeginOverlap(OtherActor)
-- end

-- function M:ReceiveActorEndOverlap(OtherActor)
-- end

-- function M:ReceivePossessed(NewController)
-- end

-- 添加一个头顶信息小部件
-- 该函数用于在游戏界面中添加一个显示玩家网络角色的头顶信息小部件
function M:AddOverheadWidget()
    -- 加载头顶信息小部件的蓝图类
    local WBP_Oh_WidgetClass = UE.UClass.Load("/Game/Blueprints/HUD/WBP_OverheadWidget.WBP_OverheadWidget_C")

    -- 获取OverheadWidget类，用于后续创建和操作头顶信息小部件
    local OverheadWidget = UE.UOverheadWidget

    -- 尝试从当前对象的头顶信息小部件属性中获取小部件实例，并将其类型转换为之前加载的蓝图类
    OverheadWidget = self.OverheadWidget:GetUserWidgetObject():Cast(WBP_Oh_WidgetClass)

    -- 如果成功获取到头顶信息小部件，则显示玩家的网络角色信息
    if OverheadWidget then
        OverheadWidget:ShowPlayerNetRole(self)
    end
end


--- 绑定输入
--function M:BindInput()
--    -- self:AddMappingContext()
--    print("哈哈")
--end

-- 添加映射上下文到增强输入系统
function M:AddMappingContext()
    -- 获取玩家控制器
    local PlayerController = self.Controller:Cast(UE.APlayerController)
    -- 检查玩家控制器是否存在
    if PlayerController then
        -- 获取增强输入系统
        EnhancedInput = UE.USubsystemBlueprintLibrary.GetLocalPlayerSubSystemFromPlayerController(PlayerController, UE.UEnhancedInputLocalPlayerSubsystem)
        -- 检查增强输入系统是否存在
        if EnhancedInput then
            -- 添加默认映射上下文
            EnhancedInput:AddMappingContext(self.IMC_Default, 0, nil)
            Screen.Print(self:GetName() .. "绑定成功！！！")
        end
    end
end


--- 移动持续按下
-- 当移动触发时执行的操作
-- 此函数用于处理角色的移动输入，基于给定的动作值来调整角色的朝向和移动
------@param ActionValue FInputActionValue
function M:Move_Triggered(ActionValue)
    -- 检查是否有控制器，如果没有则不执行任何操作
    if not self:GetController() then
        return
    end

    -- 创建一个FRotator对象，用于确定角色的朝向
    -- 通过使用控制旋转的Yaw值来设置旋转，忽略Pitch和Roll
    local Rotation = UE.FRotator(0, self:GetControlRotation().Yaw, 0)

    -- 根据角色的朝向前向添加移动输入
    -- 参数包括向前向量、动作值的X分量和是否锁定输入
    self:AddMovementInput(Rotation:GetForwardVector(), ActionValue.X, true)
    -- 根据角色的朝向右向添加移动输入
    -- 参数包括向右向量、动作值的Y分量和是否锁定输入
    self:AddMovementInput(Rotation:GetRightVector(), ActionValue.Y, true)
end

--- 跳跃按下
------@param ActionValue FInputActionValue
function M:Jump_Started(ActionValue)

    -- 检查是否具有控制器，如果没有，则不执行任何动作并退出当前函数
    if not self:GetController() then
        return
    end

    -- 根据角色的蹲伏状态，执行相应的动作：取消蹲伏或停止跳跃
    if self.bIsCrouched then
        -- 如果角色正在蹲伏，则取消蹲伏状态
        self:UnCrouch()
    else
        -- 调用Jump方法，使对象跳跃
        self:Jump()
    end

    -- 打印当前对象的速度信息
    -- print(self:GetVelocity())
end

--- 跳跃完成
------@param ActionValue FInputActionValue
function M:Jump_Completed(ActionValue)

    -- 检查是否有控制器，如果没有，则不执行任何操作并退出函数
    if not self:GetController() then
        return
    end

    self:StopJumping()
end

--- 观看动作持续按下
-- 当查看操作被触发时调用此函数
---@param ActionValue FInputActionValue 一个包含X和Y轴值的表，用于指定查看的方向
function M:Look_Triggered(ActionValue)
    -- 检查是否获取到了控制器，如果没有，则退出函数
    if not self:GetController() then
        return
    end

    -- 添加控制器的yaw输入，用于左右查看
    self:AddControllerYawInput(ActionValue.X)
    -- 添加控制器的pitch输入，用于上下查看
    self:AddControllerPitchInput(ActionValue.Y)
end

--- 蹲下按下
-- 当蹲下操作被触发时调用此函数
---@param ActionValue FInputActionValue
function M:Crouch_Started(ActionValue)
    -- 根据角色当前的状态决定是取消下蹲还是开始下蹲
    if self.bIsCrouched then
        -- 如果角色当前是下蹲状态，则取消下蹲
        self:UnCrouch()
    else
        -- 如果角色当前不是下蹲状态，并且不在下落状态，则开始下蹲
        if self.CharacterMovement:IsFalling() == false then
            self:Crouch()
        end
    end
end

return M
