
##############################################
#Tracer

target_sources(soul-engine
    PUBLIC
		FILE_SET CXX_MODULES FILES
			tracer.ixx
			camera/camera.ixx
			camera/film/film.ixx
			camera/film/filter/filter.ixx
			ray/ray.ixx
			ray/ray_job.ixx
	PRIVATE
			camera/film/film.cpp
			ray/ray_job.cpp
)

