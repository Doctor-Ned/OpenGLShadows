@echo off
for %%x in (
			"Release Basic (shadows only)"
			"Release PCF (shadows only)"
			"Release VSM (shadows only)"
			"Release PCSS (shadows only)"
			"Release CHSS (shadows only)"
			"Release Master (shadows only)"
			) do (
				echo Running %%x...
				..\x64\%%x\OpenGLShadowsExec.exe benchmark
			)