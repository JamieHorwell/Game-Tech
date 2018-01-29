

/* float3 Normalise(float3 vec) {
	
	
	return vec;
}

float3 Dot(float3 vec) {
	
	return vec;
}

float3 Length(float3 vec) {
	
	return vec;
} */

__kernel void solveConstraints(__global float* posA, __global float* posB, __global float* velA, __global float* velB, __global float* massA, __global float* massB, __global float* targetLength,  unsigned float deltaT)
{
	/* int workid = get_global_id(0);
	
		//lets try without r1,r2 first, as they are strictly for rotational constraints (i think)
		float3 globalOnA = posA[workid];
		float3 globalOnB =  posB[workid];
		
		
		float3 ab = globalOnA - globalOnB;
		//TODO: WRITE NORMALISE FUNCTION
		float3 abn = Normalise(ab);
		
		
		//No need to compute v0,v1 as not interested in angular
		
		float3 v0v1 = velA[workid] - velB[workid];
		//TODO: WRITE DOT FUNCTION
		float abnVel = Dot(v0v1, abn);
		
		
		
		float invConstraintMassLin = massA[workid] + massB[workid];
		
		if(invConstraintMassLin > 0.0f) {
			float b = 0.0f
				//TODO: WRITE LENGTH FUNCTION
			float distOffset = Length(ab) - targetLength[workid];
			float baumgarte_scalar = 0.1f;
			b = -(baumgarte_scalar / deltaT) * distOffset;
			
			float jn = (-(abnVel + b) / invConstraintMassLin) * 0.9;
			
			//set lin vel of A
			velA[workid] = velA[workid] + abn * (massA[workid] * jn);
			//set lin vel of B
			velB[workid] = velB[workid] + abn * (massB[workid] * jn);
			
		} */
		
		velA[workid] = 5;
		velB[workid] = 5;
}