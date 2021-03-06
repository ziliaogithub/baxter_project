/*
 * Based on sac_model_registration
 */

#ifndef PCL_SAMPLE_CONSENSUS_MODEL_REGISTRATION_REPROJECTION_H_
#define PCL_SAMPLE_CONSENSUS_MODEL_REGISTRATION_REPROJECTION_H_

#include "G2OStereoProjector.hpp"

namespace pcl_peter
{
  template <typename PointT, typename KeypointT>
  class SampleConsensusModelRegistrationReprojection : public pcl::SampleConsensusModel<PointT>
  {
	using pcl::SampleConsensusModel<PointT>::input_;
	using pcl::SampleConsensusModel<PointT>::indices_;

	public:
	  typedef boost::shared_ptr<SampleConsensusModelRegistrationReprojection> Ptr;

	  typedef typename pcl::SampleConsensusModel<PointT>::PointCloud PointCloud;
	  typedef typename pcl::SampleConsensusModel<PointT>::PointCloudPtr PointCloudPtr;
	  typedef typename pcl::SampleConsensusModel<PointT>::PointCloudConstPtr PointCloudConstPtr;

	  typedef typename pcl::PointCloud<KeypointT> KeypointCloud;
	  typedef typename KeypointCloud::Ptr KeypointCloudPtr;
	  typedef typename KeypointCloud::ConstPtr KeypointCloudConstPtr;

	  // forces point type to match keypoint type (uses just xyz)
	  G2OStereoProjector<PointT, KeypointT> projector_;
	  KeypointCloudConstPtr input_keypoints_;
	  KeypointCloudConstPtr target_keypoints_;
	  bool fix_point_positions_in_sba_;

	  SampleConsensusModelRegistrationReprojection (
			  const PointCloudConstPtr &cloud,
			  const KeypointCloudConstPtr &keypoint_cloud,
			  const G2OStereoProjector<PointT, KeypointT>& projector,
			  bool fix_point_positions_in_sba)
	  : pcl::SampleConsensusModel<PointT> (cloud),
		projector_(projector),
		fix_point_positions_in_sba_(fix_point_positions_in_sba)
	  {
		// Call our own setInputCloud
		setInputCloud (cloud);
		// end PCL
		input_keypoints_ = keypoint_cloud;
	  }

	  // shouldn't need index version
#if 0
	  SampleConsensusModelRegistrationReprojection (const PointCloudConstPtr &cloud,
			  const std::vector<int> &indices,
			  const g2oStereoProjector& projector)
	  : pcl::SampleConsensusModel<PointT> (cloud, indices)
	  {
		computeOriginalIndexMapping ();
		computeSampleDistanceThreshold (cloud, indices);
		// end PCL
	  }
#endif

	  /** \brief Provide a pointer to the input dataset
		* \param[in] cloud the const boost shared pointer to a PointCloud message
		*/
	  inline virtual void
	  setInputCloud (const PointCloudConstPtr &cloud)
	  {
		pcl::SampleConsensusModel<PointT>::setInputCloud (cloud);
		computeOriginalIndexMapping ();
		computeSampleDistanceThreshold (cloud);
	  }

	  /** \brief Set the input point cloud target.
		* \param target the input point cloud target
		*/
	  inline void
	  setInputTarget (
			  const PointCloudConstPtr &target,
			  const KeypointCloudConstPtr &target_keypoints
			  )
	  {
		target_ = target;
		indices_tgt_.reset (new std::vector<int>);
		// Cache the size and fill the target indices
		unsigned int target_size = target->size ();
		indices_tgt_->resize (target_size);

		for (unsigned int i = 0; i < target_size; ++i)
		  (*indices_tgt_)[i] = i;
		computeOriginalIndexMapping ();
		// end PCL
		target_keypoints_ = target_keypoints;
	  }

	  // really don't need this...really
#if 0
	  /** \brief Set the input point cloud target.
		* \param[in] target the input point cloud target
		* \param[in] indices_tgt a vector of point indices to be used from \a target
		*/
	  inline void
	  setInputTarget (const PointCloudConstPtr &target,
			  const std::vector<int> &indices_tgt)
	  {
		target_ = target;
		indices_tgt_.reset (new std::vector<int> (indices_tgt));
		computeOriginalIndexMapping ();
		fillInNormalZDisparity(*target);
	  }
#endif

	  /** \brief Compute a 4x4 rigid transformation matrix from the samples given
		* \param[in] samples the indices found as good candidates for creating a valid model
		* \param[out] model_coefficients the resultant model coefficients
		*/
	  bool
	  computeModelCoefficients (const std::vector<int> &samples,
								Eigen::VectorXf &model_coefficients);

	  /** \brief Compute all distances from the transformed points to their correspondences
		* \param[in] model_coefficients the 4x4 transformation matrix
		* \param[out] distances the resultant estimated distances
		*/
	  void
	  getDistancesToModel (const Eigen::VectorXf &model_coefficients,
						   std::vector<double> &distances);

	  /** \brief Select all the points which respect the given model coefficients as inliers.
		* \param[in] model_coefficients the 4x4 transformation matrix
		* \param[in] threshold a maximum admissible distance threshold for determining the inliers from the outliers
		* \param[out] inliers the resultant model inliers
		*/
	  void
	  selectWithinDistance (const Eigen::VectorXf &model_coefficients,
							const double threshold,
							std::vector<int> &inliers);

	  /** \brief Count all the points which respect the given model coefficients as inliers.
		*
		* \param[in] model_coefficients the coefficients of a model that we need to compute distances to
		* \param[in] threshold maximum admissible distance threshold for determining the inliers from the outliers
		* \return the resultant number of inliers
		*/
	  virtual int
	  countWithinDistance (const Eigen::VectorXf &model_coefficients,
						   const double threshold);

	  /** \brief Recompute the 4x4 transformation using the given inlier set
		* \param[in] inliers the data inliers found as supporting the model
		* \param[in] model_coefficients the initial guess for the optimization
		* \param[out] optimized_coefficients the resultant recomputed transformation
		*/
	  void
	  optimizeModelCoefficients (const std::vector<int> &inliers,
								 const Eigen::VectorXf &model_coefficients,
								 Eigen::VectorXf &optimized_coefficients);

	  void
	  projectPoints (const std::vector<int> &inliers,
					 const Eigen::VectorXf &model_coefficients,
					 PointCloud &projected_points, bool copy_data_fields = true)
	  {};

	  bool
	  doSamplesVerifyModel (const std::set<int> &indices,
							const Eigen::VectorXf &model_coefficients,
							const double threshold)
	  {
		//PCL_ERROR ("[pcl::SampleConsensusModelRegistrationReprojection::doSamplesVerifyModel] called!\n");
		return (false);
	  }

	  /** \brief Return an unique id for this model (SACMODEL_REGISTRATION). */
	  /*
	   * Peter: gotta keep the type, I think...so this is wrong
	   */
	  inline pcl::SacModel
	  getModelType () const { return (pcl::SACMODEL_REGISTRATION); } // was SACMODEL_REGISTRATION

	protected:
	  /** \brief Check whether a model is valid given the user constraints.
		* \param[in] model_coefficients the set of model coefficients
		*/
	  inline bool
	  isModelValid (const Eigen::VectorXf &model_coefficients)
	  {
		// Needs a valid model coefficients
		if (model_coefficients.size () != 16)
		  return (false);

		return (true);
	  }

	  /** \brief Check if a sample of indices results in a good sample of points
		* indices.
		* \param[in] samples the resultant index samples
		*/
	  bool
	  isSampleGood (const std::vector<int> &samples) const;

	  /** \brief Computes an "optimal" sample distance threshold based on the
		* principal directions of the input cloud.
		* \param[in] cloud the const boost shared pointer to a PointCloud message
		*/
	  inline void
	  computeSampleDistanceThreshold (const PointCloudConstPtr &cloud)
	  {
		// Compute the principal directions via PCA
		Eigen::Vector4f xyz_centroid;
		Eigen::Matrix3f covariance_matrix = Eigen::Matrix3f::Zero ();

		computeMeanAndCovarianceMatrix (*cloud, covariance_matrix, xyz_centroid);

		// Check if the covariance matrix is finite or not.
		for (int i = 0; i < 3; ++i)
		  for (int j = 0; j < 3; ++j)
			if (!pcl_isfinite (covariance_matrix.coeffRef (i, j)))
			  PCL_ERROR ("[pcl::SampleConsensusModelRegistrationReprojection::computeSampleDistanceThreshold] Covariance matrix has NaN values! Is the input cloud finite?\n");

		Eigen::Vector3f eigen_values;
		pcl::eigen33 (covariance_matrix, eigen_values);

		// Compute the distance threshold for sample selection
		sample_dist_thresh_ = eigen_values.array ().sqrt ().sum () / 3.0;
		sample_dist_thresh_ *= sample_dist_thresh_;
		PCL_DEBUG ("[pcl::SampleConsensusModelRegistrationReprojection::setInputCloud] Estimated a sample selection distance threshold of: %f\n", sample_dist_thresh_);
	  }

	  /** \brief Computes an "optimal" sample distance threshold based on the
		* principal directions of the input cloud.
		* \param[in] cloud the const boost shared pointer to a PointCloud message
		*/
	  inline void
	  computeSampleDistanceThreshold (const PointCloudConstPtr &cloud,
									  const std::vector<int> &indices)
	  {
		// Compute the principal directions via PCA
		Eigen::Vector4f xyz_centroid;
		Eigen::Matrix3f covariance_matrix;
		computeMeanAndCovarianceMatrix (*cloud, indices, covariance_matrix, xyz_centroid);

		// Check if the covariance matrix is finite or not.
		for (int i = 0; i < 3; ++i)
		  for (int j = 0; j < 3; ++j)
			if (!pcl_isfinite (covariance_matrix.coeffRef (i, j)))
			  PCL_ERROR ("[pcl::SampleConsensusModelRegistrationReprojection::computeSampleDistanceThreshold] Covariance matrix has NaN values! Is the input cloud finite?\n");

		Eigen::Vector3f eigen_values;
		pcl::eigen33 (covariance_matrix, eigen_values);

		// Compute the distance threshold for sample selection
		sample_dist_thresh_ = eigen_values.array ().sqrt ().sum () / 3.0;
		sample_dist_thresh_ *= sample_dist_thresh_;
		PCL_DEBUG ("[pcl::SampleConsensusModelRegistrationReprojection::setInputCloud] Estimated a sample selection distance threshold of: %f\n", sample_dist_thresh_);
	  }

	private:

	/** \brief Estimate a rigid transformation between a source and a target point cloud using an SVD closed-form
	  * solution of absolute orientation using unit quaternions
	  * \param[in] cloud_src the source point cloud dataset
	  * \param[in] indices_src the vector of indices describing the points of interest in cloud_src
	  * \param[in] cloud_tgt the target point cloud dataset
	  * \param[in] indices_tgt the vector of indices describing the correspondences of the interest points from
	  * indices_src
	  * \param[out] transform the resultant transformation matrix (as model coefficients)
	  *
	  * This method is an implementation of: Horn, B. “Closed-Form Solution of Absolute Orientation Using Unit Quaternions,” JOSA A, Vol. 4, No. 4, 1987
	  */
	  void
	  estimateRigidTransformationSVD (const pcl::PointCloud<PointT> &cloud_src,
									  const std::vector<int> &indices_src,
									  const pcl::PointCloud<PointT> &cloud_tgt,
									  const std::vector<int> &indices_tgt,
									  Eigen::VectorXf &transform);

	  /** \brief Compute mappings between original indices of the input_/target_ clouds. */
	  void
	  computeOriginalIndexMapping ()
	  {
		if (!indices_tgt_ || !indices_ || indices_->empty () || indices_->size () != indices_tgt_->size ())
		  return;
		for (size_t i = 0; i < indices_->size (); ++i)
		  correspondences_[(*indices_)[i]] = (*indices_tgt_)[i];
	  }

	  /** \brief A boost shared pointer to the target point cloud data array. */
	  PointCloudConstPtr target_;

	  /** \brief A pointer to the vector of target point indices to use. */
	  boost::shared_ptr <std::vector<int> > indices_tgt_;

	  /** \brief Given the index in the original point cloud, give the matching original index in the target cloud */
	  boost::unordered_map<int, int> correspondences_;

	  /** \brief Internal distance threshold used for the sample selection step. */
	  double sample_dist_thresh_;
	public:
	  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };
}

#include "sac_model_registration_reprojection.hpp"

#endif  //#ifndef PCL_SAMPLE_CONSENSUS_MODEL_REGISTRATION_REPROJECTION_H_
