@echo off
for %%x in (
			"Release Basic (shadows only)"
			"Release Basic"
			"Release PCF (shadows only)"
			"Release PCF"
			"Release VSM (shadows only)"
			"Release VSM"
			"Release PCSS (shadows only)"
			"Release PCSS"
			"Release CHSS (shadows only)"
			"Release CHSS"
			"Release Master (shadows only)"
			"Release Master"
			) do (
				echo Generating screenshots for %%x...
				..\x64\%%x\OpenGLShadowsExec.exe screenshots
			)