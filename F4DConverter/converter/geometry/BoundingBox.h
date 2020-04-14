#pragma once

#include <vector>

namespace gaia3d
{
	class BoundingBox
	{
	public:
		BoundingBox();

		virtual ~BoundingBox();

	public:
		double minX, minY, minZ, maxX, maxY, maxZ;
		
		///< init 함수를 실행시키고 난 후 이 변수가 true로 변함
		bool isInitialized;

		///< 기존 박스에 새로운 박스를 포함시켜 새로운 경계를 갱신
		void addBox(BoundingBox& bbox);

		///< 기존 박스에 새로운 포인트를 포함시켜 새로운 경계를 갱신
		void addPoint(double x, double y, double z);

		///< 박스의 중심점을 계산
		void getCenterPoint(double& x, double& y, double& z);

		///< 박스의 제일 긴 변을 계산
		double getMaxLength();

		///< 박스의 x축과 나란한 변의 길이를 계산
		double getXLength();

		///< 박스의 y축과 나란한 변의 길이를 계산
		double getYLength();

		///< 박스의 z축과 나란한 변의 길이를 계산
		double getZLength();

		///< 박스의 점들을 해당 점으로 초기화
		void init(double x, double y, double z);

		///< 박스의 변을 edgeDivisionCount만큼 나눠서 result로 리턴
		void divideBbox(int edgeDivisionCount, std::vector<BoundingBox>& result);

		///< 박스의 가로세로높이를 dist*2만큼 확장
		void expand(double dist);
	};
}