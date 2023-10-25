@echo off
setlocal enabledelayedexpansion
SET PENGUIN_EXE=Penguin.exe
SET EXAMPLES_DIR=examples

for /D %%d in (%EXAMPLES_DIR%\*) do (
    SET PENG_FILE=%%d\%%~nd.peng

    if exist "%%d\%%~nd.peng" (
        for %%i in (%%d\in\*.in) do (
            SET BASE_NAME=%%~ni
            SET OUT_FILE=%%d\out\!BASE_NAME!.out
            SET TEMP_OUT=%%d\out\!BASE_NAME!_temp.out

            %PENGUIN_EXE% "!PENG_FILE!" < "%%i" > "!TEMP_OUT!"

            fc "!TEMP_OUT!" "!OUT_FILE!" >nul
            if errorlevel 1 (
                echo Test %%i for !PENG_FILE! failed.
            ) else (
                echo Test %%i for !PENG_FILE! passed.
            )

            del "!TEMP_OUT!"
        )
        echo.
    )
)
endlocal
pause
exit /b