define_property(
	TARGET 
	PROPERTY WHOLE_ARCHIVE 
	BRIEF_DOCS "Links target to all executables with the whole-archive linker option."
	FULL_DOCS "Links target to all executables with the whole-archive linker option. Add some more docs asd."
)

function(make_archive_whole archive_name archive_output)
	# This is just a hack for now.
	# TODO: handle generators and compilers other than VS and MSVC.
	if (TARGET_COMPILER_MSVC)
		if (GENERATOR_IS_MULTI_CONFIG)
			set(${archive_output} "-WHOLEARCHIVE:${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$(Configuration)/${archive_name}" PARENT_SCOPE)
		else()
			set(${archive_output} "-WHOLEARCHIVE:${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${archive_name}" PARENT_SCOPE)
		endif()
	else()
		message(SEND_ERROR "Whole archives are not written for compiler & generator.")
	endif()
endfunction()


function(target_whole_archives)
    cmake_parse_arguments(PARSED_ARGS "" "OUT" "ARCHIVES;WHOLES" ${ARGN})

	set(stripped_in ${PARSED_ARGS_ARCHIVES})
	set(add_as_whole)

	foreach(arch IN ITEMS ${PARSED_ARGS_WHOLES})
		if (${arch} IN_LIST stripped_in)
			LIST(REMOVE_ITEM stripped_in ${arch})
			LIST(APPEND add_as_whole ${arch})
		endif()
	endforeach()

	set(wholified)
	foreach(whole_arch IN ITEMS ${add_as_whole}) 
		make_archive_whole(${whole_arch} cmdline_option)
		LIST(APPEND wholified ${cmdline_option})
	endforeach()

	set(new_archives ${stripped_in} ${wholified})
	set(${PARSED_ARGS_OUT} ${new_archives} PARENT_SCOPE)
endfunction()


function(set_whole_archive_linkage _dir)
	get_all_targets(all_targets, ${_dir})
	set(exe_targets)
	set(whole_archives)

	foreach (tar IN ITEMS ${all_targets})
		get_target_property(tar_type ${tar} TYPE)
		get_target_property(is_whole ${tar} WHOLE_ARCHIVE)
		if (tar_type MATCHES "(SHARED_LIBRARY|EXECUTABLE)")
			list(APPEND exe_targets ${tar})
		elseif (tar_type MATCHES "STATIC_LIBRARY" AND is_whole)
			list(APPEND whole_archives ${tar})
		endif()
	endforeach()	

	foreach (tar IN ITEMS ${exe_targets})
		get_target_property(linker_input ${tar} LINK_LIBRARIES)
		get_target_property(target_name ${tar} NAME)
		target_whole_archives(ARCHIVES ${linker_input} WHOLES ${whole_archives} OUT new_archives)
		set_property(TARGET ${tar} PROPERTY LINK_LIBRARIES ${new_archives})
	endforeach()
message(STATUS ${OUT})
endfunction()
