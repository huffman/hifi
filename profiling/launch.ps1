# if "%1"=="" goto blank

$CONTENT_SET=$Args[0]

echo "Content set is: $CONTENT_SET"
echo "HOME: $HOME"

$INTERFACE_BIN="$HOME\dev\hifi\build\interface\Release\interface.exe"
$DS_BIN="$HOME\dev\hifi\build\domain-server\Release\domain-server.exe"
$AC_BIN="$HOME\dev\hifi\build\assignment-client\Release\assignment-client.exe"

$DS_CONFIG="$APPDATA\High Fidelity - dev\content_sets\$CONTENT_SET\domain-server\config.json"

echo $DS_BIN
echo $DS_CONFIG

& $DS_BIN
start /b "domain-server" "$DS_BIN" --user-config "$DS_CONFIG"
start /b "ac" $AC_BIN "-t1" | out-null
start /b "ac" $AC_BIN "-t2" > $null
start /b "ac" $AC_BIN "-t3" > $null
start /b "ac" $AC_BIN "-t6" > $null

# sleep for 10 seconds
# ping 192.0.2.2 -n 1 -w 40000 > $null

$TRACE_FILENAME="F:/trace_%CONTENT_SET%.json"
# taskkill /t /pid $PID
#
# S
#
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

# start client
# $INTERFACE_BIN --trace $TRACE_FILENAME --duration 10000
