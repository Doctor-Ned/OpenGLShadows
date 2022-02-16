for %%x in (
			"Release Basic"
			"Release Basic (shadows only)"
			"Release PCF"
			"Release PCF (shadows only)"
			"Release VSM"
			"Release VSM (shadows only)"
			"Release PCSS"
			"Release PCSS (shadows only)"
			"Release CHSS"
			"Release CHSS (shadows only)"
			"Release Master"
			"Release Master (shadows only)"
			) do (
				echo Running '%%x'
				..\x64\%%x\OpenGLShadowsExec.exe benchmark
			)