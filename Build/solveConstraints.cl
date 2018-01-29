



float Dot(float3 vec, float3 vec2) {
	return (vec.x*vec2.x) + (vec.y*vec2.y) + (vec.z*vec2.z);
}

float Length(float3 vec) {
	return sqrt((vec.x*vec.x)+(vec.y*vec.y)+(vec.z*vec.z));
}

float3 Normalise(float3 vec) {
	float length = Length(vec);
	
	float3 newVec;
	
	if(length != 0.0f) {
		length = 1.0f / length;
		newVec.x = vec.x * length;
		newVec.y = vec.y * length;
		newVec.z = vec.z * length;
	}
	return newVec;
}

__kernel void solveConstraints(__global float3* posA, __global float3* posB, __global float3* velA, __global float3* velB, __global float* massA, __global float* massB, __global float* targetLength,  float deltaT, __global float3* velInA, __global float3* velInB, int last)
{
	int workid = get_global_id(0);
	float3 globalOnA = posA[workid];
	float3 globalOnB =  posB[workid];
	
	float3 ab = globalOnB - globalOnA;
	
	float3 abn = Normalise(ab);
	
	
	float3 v0v1 = velInA[workid] - velInB[workid];
	float abnVel = Dot(v0v1, abn);
	
	float invConstraintMassLin = massA[workid] + massB[workid];
	
	
	
	
	
	if(invConstraintMassLin > 0.0f) {
			float b = 0.0f;
				//TODO: WRITE LENGTH FUNCTION
			float distOffset = Length(ab) - targetLength[workid];
			float baumgarte_scalar = 0.1f;
			b = -(baumgarte_scalar / 0.013) * distOffset;
			
			float jn = (-(abnVel + b) / invConstraintMassLin) *0.2;
			
			//velA[workid] = (float3)(0,0,0);
			//velB[workid] = (float3)(0,0,0);
			
			velA[workid] = (abn * (massA[workid] * jn));
			velB[workid] = (abn * (massB[workid] * jn));	
			
			//velInA[workid] += velA[workid];
			//velInB[workid] -= velB[workid];
	}
	
	

	/*  if(last == 1) {
		velA[workid] = velInA[workid];
		velB[workid] = -velInB[workid];
	}  */
	
}