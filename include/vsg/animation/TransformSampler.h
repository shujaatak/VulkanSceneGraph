#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/Animation.h>
#include <vsg/app/ViewMatrix.h>
#include <vsg/maths/transform.h>

namespace vsg
{

    struct VectorKey
    {
        double time;
        dvec3 value;

        bool operator<(const VectorKey& rhs) const { return time < rhs.time; }
    };

    struct QuatKey
    {
        double time;
        dquat value;

        bool operator<(const QuatKey& rhs) const { return time < rhs.time; }
    };

    class VSG_DECLSPEC TransformKeyframes : public Inherit<Object, TransformKeyframes>
    {
    public:
        TransformKeyframes();

        /// name of node
        std::string name;

        /// position key frames
        std::vector<VectorKey> positions;

        /// rotation key frames
        std::vector<QuatKey> rotations;

        /// scale key frames
        std::vector<VectorKey> scales;

        void clear()
        {
            positions.clear();
            rotations.clear();
            scales.clear();
        }

        void add(double time, const dvec3& position, const dquat& rotation)
        {
            positions.push_back(VectorKey{time, position});
            rotations.push_back(QuatKey{time, rotation});
        }

        void add(double time, const dvec3& position, const dquat& rotation, const dvec3& scale)
        {
            positions.push_back(VectorKey{time, position});
            rotations.push_back(QuatKey{time, rotation});
            scales.push_back(VectorKey{time, scale});
        }

        void read(Input& input) override;
        void write(Output& output) const override;
    };
    VSG_type_name(vsg::TransformKeyframes);

    /// Animation sampler for sampling position, rotation and scale keyframes for setting transforms/joints.
    class VSG_DECLSPEC TransformSampler : public Inherit<AnimationSampler, TransformSampler>
    {
    public:
        TransformSampler();
        TransformSampler(const TransformSampler& rhs, const CopyOp& copyop = {});

        ref_ptr<TransformKeyframes> keyframes;
        ref_ptr<Object> object;

        // updated using keyFrames
        dvec3 position;
        dquat rotation;
        dvec3 scale;

        void update(double time) override;
        double maxTime() const override;

        inline dmat4 transform() const { return translate(position) * vsg::rotate(rotation) * vsg::scale(scale); }

    public:
        ref_ptr<Object> clone(const CopyOp& copyop = {}) const override { return TransformSampler::create(*this, copyop); }
        int compare(const Object& rhs) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        void apply(mat4Value& mat) override;
        void apply(dmat4Value& mat) override;
        void apply(MatrixTransform& mt) override;
        void apply(Joint& joint) override;
        void apply(LookAt& lookAt) override;
        void apply(Camera& camera) override;
    };
    VSG_type_name(vsg::TransformSampler);

} // namespace vsg
