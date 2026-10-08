param (
    [int]$Port = 9999
)

try {
    $client = New-Object System.Net.Sockets.TcpClient("127.0.0.1", $Port)
    $stream = $client.GetStream()
    $writer = New-Object System.IO.StreamWriter($stream)
    $reader = New-Object System.IO.StreamReader($stream)
    $writer.AutoFlush = $true

    Write-Host "Connected to MiniRedis on port $Port!" -ForegroundColor Green
    Write-Host "Type commands (e.g. PING, SET foo bar, GET foo) or 'quit' to exit:`n"

    while ($true) {
        $cmd = Read-Host "127.0.0.1:$Port"
        if ([string]::IsNullOrWhiteSpace($cmd)) {
            continue
        }
        if ($cmd.Trim().ToLower() -in @("quit", "exit")) {
            Write-Host "Disconnecting..."
            break
        }

        $writer.WriteLine($cmd)
        $response = $reader.ReadLine()
        if ($null -eq $response) {
            Write-Host "Server closed connection." -ForegroundColor Yellow
            break
        }
        Write-Host $response
    }
}
catch {
    Write-Host "Connection error: $_" -ForegroundColor Red
}
finally {
    if ($client) { $client.Close() }
}
