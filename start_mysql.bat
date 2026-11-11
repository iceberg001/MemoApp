@echo off
REM 启动本地 MySQL（供 MemoApp 周报功能使用）
cd /d "%~dp0third_party\mysql"
if not exist mysql_data (
    echo [INFO] 初始化 MySQL 数据目录...
    bin\mysqld.exe --no-defaults --initialize-insecure --basedir=. --datadir=./mysql_data
)
echo [INFO] 启动 MySQL (127.0.0.1:3306, root 空密码)...
start "mysqld" /B bin\mysqld.exe --no-defaults --datadir=%~dp0third_party\mysql\mysql_data --port=3306
timeout /t 3 >nul
bin\mysql.exe --no-defaults -h127.0.0.1 -uroot -e "CREATE DATABASE IF NOT EXISTS memo_app DEFAULT CHARACTER SET utf8mb4;"
echo [OK] MySQL 已启动，数据库 memo_app 就绪
