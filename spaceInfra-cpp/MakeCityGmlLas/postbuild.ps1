$base_dir=split-path -parent $MyInvocation.MyCommand.Definition
$arg0=$Args[0]
$out_dir="${arg0}"

Try {
	if ($base_dir -And $out_dir -And (Test-Path -Path $base_dir) -And (Test-Path -Path $out_dir)) {
		$citygml_lib_dir = "${base_dir}\libcitygml\lib\"
		
		if (!(Test-Path "${out_dir}citygml.dll")) {
			Copy-Item "${citygml_lib_dir}citygml.dllx" -Destination "${out_dir}citygml.dll"
		}
		if (!(Test-Path "${out_dir}xerces-c_3_2.dll")) {
			Copy-Item "${citygml_lib_dir}xerces-c_3_2.dllx" -Destination "${out_dir}xerces-c_3_2.dll"
		}
		
		Exit 0
	}
}
Catch {
	Exit 9
}
