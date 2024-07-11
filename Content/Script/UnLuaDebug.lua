local Path = UE.UKismetSystemLibrary:GetProjectDirectory()
Path = ";" .. Path .. "Emmy/?.dll"
package.cpath = package.cpath .. Path
local dbg = require('emmy_core')
dbg.tcpConnect('localhost', 9966)