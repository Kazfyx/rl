//
// Copyright (c) 2009, Markus Rickert
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <array>
#include <QMainWindow>
#include <QTableView>
#include <rl/math/Quaternion.h>
#include <rl/math/Rotation.h>
#include <rl/math/Vector.h>

class AngleAxisModel;
class EulerAnglesModel;
class QuaternionModel;
class RotationMatrixModel;

class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	virtual ~MainWindow();
	
	static MainWindow* instance();
	
public slots:
	void angleAxisChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
	
	void eulerAnglesChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
	
	void inputEulerAxesChanged(int index);
	
	void inputUnitChanged(int index);
	
	void outputEulerAxesChanged(int index);
	
	void outputUnitChanged(int index);
	
	void quaternionChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
	
	void rotationMatrixChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
	
protected:
	MainWindow(QWidget* parent = nullptr, Qt::WindowFlags f = 0);
	
private:
	static std::size_t EULER_ANGLES[12][3];
	
	rl::math::AngleAxis inputAngleAxis;
	
	AngleAxisModel* inputAngleAxisModel;
	
	rl::math::Vector3 inputEulerAngles;
	
	EulerAnglesModel* inputEulerAnglesModel;
	
	std::array<std::size_t, 3> inputEulerAxes;
	
	rl::math::Quaternion inputQuaternion;
	
	rl::math::Rotation inputRotationMatrix;
	
	bool inputUnitRadians;
	
	rl::math::AngleAxis outputAngleAxis;
	
	AngleAxisModel* outputAngleAxisModel;
	
	rl::math::Vector3 outputEulerAngles;
	
	EulerAnglesModel* outputEulerAnglesModel;
	
	std::array<std::size_t, 3> outputEulerAxes;
	
	rl::math::Quaternion outputQuaternion;
	
	QuaternionModel* outputQuaternionModel;
	
	rl::math::Rotation outputRotationMatrix;
	
	RotationMatrixModel* outputRotationMatrixModel;
	
	bool outputUnitRadians;
	
	static MainWindow* singleton;
};

#endif // MAINWINDOW_H
