#pragma once

#include "Core/Scene/Scene.h"
#include "RayJob.h"
//#include "Parallelism/ComputeOld/ComputeBuffer.h"
//#include "Core/Utility/Timer/Timer.h"
//#include "Ray.h"
//
//#undef AddJob
//#undef GetJob
//
////The main engine that processes RayJobs
//class RayEngine {
//
//public:
//
//	static RayEngine& Instance()
//	{
//
//		static RayEngine instance; 
//		return instance;
//	}
//
//
//	RayEngine(const RayEngine&) = delete;
//	void operator=(const RayEngine&) = delete;
//	/*
//	 *    Adds a job to be executed after all updates have taken place. It will execute the given
//	 *    function to initialize all its rays and returns the tag that can be used to extract the
//	 *    data in 'UpdateLate'.
//	 *    @param 		 	parameter1	The first parameter.
//	 *    @param 		 	parameter2	The second parameter.
//	 *    @param 		 	parameter3	True to parameter 3.
//	 *    @param 		 	parameter4	The fourth parameter.
//	 *    @param 		 	parameter5	The fifth parameter.
//	 *    @param [in,out]	parameter6	If non-null, the parameter 6.
//	 *    @param [in,out]	parameter7	If non-null, the parameter 7.
//	 *    @return	Null if it fails, else a pointer to a RayJob.
//	 */
//
//	uint AddJob(rayType, bool, float);
//
//	//A varient that does not copy the results to the CPU but instead returns a cuda* that can be procesed further.
//	//adds a job with a hint to keep its allocated data for ray storage. Speed gains if large ray bundles are given.
//	//RayJob* AddRecurringRayJob(rayType, uint, uint, Camera*);
//
//	/*
//	 *    amount cant be changed as it effects storage, but all else can;
//	 *    return false if it wasn't changed.
//	 *    @param [in,out]	parameter1	If non-null, the first parameter.
//	 *    @return	True if it succeeds, false if it fails.
//	 */
//
//	bool RemoveJob(uint);
//
//	/*
//	 *    Modify job.
//	 *    @param [in,out]	parameter1	If non-null, the first parameter.
//	 *    @param [in,out]	parameter2	The second parameter.
//	 */
//
//	RayJob& GetJob(uint);
//
//	/*
//	 *    Process this object.
//	 *    @param	parameter1	The first parameter.
//	 *    @param	parameter2	The second parameter.
//	 */
//
//	void Process(Scene&, double);
//
//	void Update();
//
//	void PreProcess();
//	void PostProcess();
//
//private:
//
//	/* List of jobs */
//	ComputeBuffer<RayJob> jobList;
//
//	/* The render derivatives */
//	std::deque<double> renderDerivatives;
//
//	/* The old render time */
//	double oldRenderTime;
//
//	/* The frame hold */
//	uint frameHold;
//
//	/* timer. */
//	Timer renderTimer;
//
//	ComputeBuffer<Ray> deviceRaysA;
//	ComputeBuffer<Ray> deviceRaysB;
//
//	//ComputeBuffer<curandState> randomState;
//
//	uint raySeedGl;
//
//	const uint rayDepth;
//
//	//stored counters
//	ComputeBuffer<int> counter;
//	ComputeBuffer<int> hitAtomic;
//
//	GPUExecutePolicy persistantPolicy;
//
//	RayEngine();
//
//	void UpdateJobs(double, double, ComputeBuffer<RayJob>&);
//
//	uint WangHash();
//
//};