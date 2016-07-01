if "%1"=="" goto blank

set CONTENT_SET=%1

echo "Content set is: %CONTENT_SET%"

set INTERFACE_BIN="%HOMEPATH%/dev/hifi/build/interface/Release/interface.exe"
set DS_BIN="%HOMEPATH%/dev/hifi/build/domain-server/Release/domain-server.exe"
set AC_BIN="%HOMEPATH%/dev/hifi/build/assignment-client/Release/assignment-client.exe"

set DS_CONFIG="%APPDATA%/High Fidelity - dev\content_sets\%CONTENT_SET%\domain-server\config.json"

start /b "domain-server" %DS_BIN% --user-config %DS_CONFIG% > nul
start /b "ac" %AC_BIN% "-t1" > nul
start /b "ac" %AC_BIN% "-t2" > nul
start /b "ac" %AC_BIN% "-t3" > nul
start /b "ac" %AC_BIN% "-t6" > nul

rem sleep for 10 seconds
rem ping 192.0.2.2 -n 1 -w 40000 > nul

set TRACE_FILENAME="F:/trace_%CONTENT_SET%.json"
rem taskkill /t /pid $PID
rem
rem S
rem
function Find-ChildProcess {
param($ID=$PID)

$CustomColumnID = @{
Name = 'Id'
Expression = { [Int[]]$_.ProcessID }
}

$result = Get-WmiObject -Class Win32_Process -Filter "ParentProcessID=$ID" |
Select-Object -Property ProcessName, $CustomColumnID, CommandLine

$result
$result | Where-Object { $_.ID -ne $null } | ForEach-Object {
Find-ChildProcess -id $_.Id
}
}
Find-ChildProcess

rem start client
rem %INTERFACE_BIN% --trace %TRACE_FILENAME% --duration 10000
