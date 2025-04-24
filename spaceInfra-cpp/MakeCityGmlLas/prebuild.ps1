$base_dir=split-path -parent $MyInvocation.MyCommand.Definition

Try {
	if ($base_dir) {
		$citygml_lib_dir = "${base_dir}\libcitygml\lib\"
		if (!(Test-Path "${citygml_lib_dir}citygml.lib")) {
			Copy-Item ${citygml_lib_dir}citygml.libx -Destination ${citygml_lib_dir}citygml.lib
		}
			
		Exit 0
	}

	Exit 9
}
Catch {
	Exit 9
}
