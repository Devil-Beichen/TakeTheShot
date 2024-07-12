local M = {}

local PrintString = UE.UKismetSystemLibrary.PrintString

function M.Print(Text, Color, Duration)
    Color = Color or UE.FLinearColor(1, 1, 1, 1)
    Duration = Duration or 100
    PrintString(nil, Text, true, true, Color, Duration)
end

return M