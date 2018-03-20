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

#include <QGridLayout>
#include <QComboBox>
#include <QGroupBox>
#include <QHeaderView>
#include <QScrollBar>
#include <rl/math/Vector.h>

#include "AngleAxisModel.h"
#include "EulerAnglesModel.h"
#include "MainWindow.h"
#include "QuaternionModel.h"
#include "RotationMatrixModel.h"
#include "TableView.h"

MainWindow::MainWindow(QWidget* parent, Qt::WindowFlags f) :
	QMainWindow(parent, f),
	inputAngleAxis(0, rl::math::Vector3::UnitZ()),
	inputAngleAxisModel(new AngleAxisModel(this)),
	inputEulerAngles(rl::math::Vector3::Zero()),
	inputEulerAnglesModel(new EulerAnglesModel(this)),
	inputEulerAxes(),
	inputQuaternion(rl::math::Quaternion::Identity()),
	inputRotationMatrix(rl::math::Rotation::Identity()),
	inputUnitRadians(false),
	outputAngleAxis(0, rl::math::Vector3::UnitZ()),
	outputAngleAxisModel(new AngleAxisModel(this)),
	outputEulerAngles(rl::math::Vector3::Zero()),
	outputEulerAnglesModel(new EulerAnglesModel(this)),
	outputEulerAxes(),
	outputQuaternion(rl::math::Quaternion::Identity()),
	outputQuaternionModel(new QuaternionModel(this)),
	outputRotationMatrix(rl::math::Rotation::Identity()),
	outputRotationMatrixModel(new RotationMatrixModel(this)),
	outputUnitRadians(false)
{
	MainWindow::singleton = this;
	
	this->inputEulerAxes[0] = 0;
	this->inputEulerAxes[1] = 1;
	this->inputEulerAxes[2] = 2;
	
	this->outputEulerAxes[0] = 0;
	this->outputEulerAxes[1] = 1;
	this->outputEulerAxes[2] = 2;
	
	QWidget* centralWidget = new QWidget(this);
	this->setCentralWidget(centralWidget);
	
	QGridLayout* gridLayout = new QGridLayout(centralWidget);
	
	// settings
	
	QGroupBox* inputSettingsGroupBox = new QGroupBox(this);
	inputSettingsGroupBox->setFlat(true);
	inputSettingsGroupBox->setTitle("Settings (Input)");
	gridLayout->addWidget(inputSettingsGroupBox, gridLayout->rowCount(), 0);
	
	QComboBox* inputUnitComboBox = new QComboBox(this);
	inputUnitComboBox->addItem("Degrees");
	inputUnitComboBox->addItem("Radians");
	
	QComboBox* inputEulerAxesComboBox = new QComboBox(this);
	
	QHBoxLayout* inputSettingsLayout = new QHBoxLayout(inputSettingsGroupBox);
	inputSettingsLayout->addWidget(inputUnitComboBox);
	inputSettingsLayout->addWidget(inputEulerAxesComboBox);
	
	QObject::connect(
		inputUnitComboBox,
		SIGNAL(currentIndexChanged(int)),
		this,
		SLOT(inputUnitChanged(int))
	);
	
	QObject::connect(
		inputEulerAxesComboBox,
		SIGNAL(currentIndexChanged(int)),
		this,
		SLOT(inputEulerAxesChanged(int))
	);
	
	QGroupBox* outputSettingsGroupBox = new QGroupBox(this);
	outputSettingsGroupBox->setFlat(true);
	outputSettingsGroupBox->setTitle("Settings (Output)");
	gridLayout->addWidget(outputSettingsGroupBox, gridLayout->rowCount() - 1, 1);
	
	QComboBox* outputUnitComboBox = new QComboBox(this);
	outputUnitComboBox->addItem("Degrees");
	outputUnitComboBox->addItem("Radians");
	
	QComboBox* outputEulerAxesComboBox = new QComboBox(this);
	
	QHBoxLayout* outputSettingsLayout = new QHBoxLayout(outputSettingsGroupBox);
	outputSettingsLayout->addWidget(outputUnitComboBox);
	outputSettingsLayout->addWidget(outputEulerAxesComboBox);
	
	QObject::connect(
		outputUnitComboBox,
		SIGNAL(currentIndexChanged(int)),
		this,
		SLOT(outputUnitChanged(int))
	);
	
	QObject::connect(
		outputEulerAxesComboBox,
		SIGNAL(currentIndexChanged(int)),
		this,
		SLOT(outputEulerAxesChanged(int))
	);
	
	for (std::size_t i = 0; i < 12; ++i)
	{
		QString text = "Euler ";
		
		for (std::size_t j = 0; j < 3; ++j)
		{
			if (j > 0)
			{
				text += "-";
			}
			
			switch (MainWindow::EULER_ANGLES[i][j])
			{
			case 0:
				text += "X";
				break;
			case 1:
				text += "Y";
				break;
			case 2:
				text += "Z";
				break;
			default:
				break;
			}
		}
		
		inputEulerAxesComboBox->addItem(text);
		outputEulerAxesComboBox->addItem(text);
	}
	
	// rotation matrix
	
	QGroupBox* inputRotationMatrixGroupBox = new QGroupBox(this);
	inputRotationMatrixGroupBox->setFlat(true);
	inputRotationMatrixGroupBox->setTitle("Rotation Matrix (Input)");
	gridLayout->addWidget(inputRotationMatrixGroupBox, gridLayout->rowCount(), 0);
	
	RotationMatrixModel* inputRotationMatrixModel = new RotationMatrixModel(this);
	inputRotationMatrixModel->rotation = &this->inputRotationMatrix;
	
	TableView* inputRotationMatrixTableView = new TableView(this);
	inputRotationMatrixTableView->setModel(inputRotationMatrixModel);
	inputRotationMatrixTableView->verticalHeader()->setMinimumWidth(20);
	
	QVBoxLayout* inputRotationMatrixLayout = new QVBoxLayout(inputRotationMatrixGroupBox);
	inputRotationMatrixLayout->addWidget(inputRotationMatrixTableView);
	
	QObject::connect(
		inputRotationMatrixModel,
		SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
		this,
		SLOT(rotationMatrixChanged(const QModelIndex&, const QModelIndex&))
	);
	
	QGroupBox* outputRotationMatrixGroupBox = new QGroupBox(this);
	outputRotationMatrixGroupBox->setFlat(true);
	outputRotationMatrixGroupBox->setTitle("Rotation Matrix (Output)");
	gridLayout->addWidget(outputRotationMatrixGroupBox, gridLayout->rowCount() - 1, 1);
	
	outputRotationMatrixModel->rotation = &this->outputRotationMatrix;
	
	TableView* outputRotationMatrixTableView = new TableView(this);
	outputRotationMatrixTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	outputRotationMatrixTableView->setModel(outputRotationMatrixModel);
	outputRotationMatrixTableView->verticalHeader()->setMinimumWidth(20);
	
	QVBoxLayout* outputRotationMatrixLayout = new QVBoxLayout(outputRotationMatrixGroupBox);
	outputRotationMatrixLayout->addWidget(outputRotationMatrixTableView);
	
	// angle axis
	
	QGroupBox* inputAngleAxisGroupBox = new QGroupBox(this);
	inputAngleAxisGroupBox->setFlat(true);
	inputAngleAxisGroupBox->setTitle("Angle Axis (Input)");
	gridLayout->addWidget(inputAngleAxisGroupBox, gridLayout->rowCount(), 0);
	
	inputAngleAxisModel->angleAxis = &this->inputAngleAxis;
	inputAngleAxisModel->angleRadians = &this->inputUnitRadians;
	
	TableView* inputAngleAxisTableView = new TableView(this);
	inputAngleAxisTableView->setModel(inputAngleAxisModel);
	inputAngleAxisTableView->verticalHeader()->setMinimumWidth(20);
	
	QVBoxLayout* inputAngleAxisLayout = new QVBoxLayout(inputAngleAxisGroupBox);
	inputAngleAxisLayout->addWidget(inputAngleAxisTableView);
	
	QObject::connect(
		inputAngleAxisModel,
		SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
		this,
		SLOT(angleAxisChanged(const QModelIndex&, const QModelIndex&))
	);
	
	QGroupBox* outputAngleAxisGroupBox = new QGroupBox(this);
	outputAngleAxisGroupBox->setFlat(true);
	outputAngleAxisGroupBox->setTitle("Angle Axis (Output)");
	gridLayout->addWidget(outputAngleAxisGroupBox, gridLayout->rowCount() - 1, 1);
	
	outputAngleAxisModel->angleAxis = &this->outputAngleAxis;
	outputAngleAxisModel->angleRadians = &this->outputUnitRadians;
	
	TableView* outputAngleAxisTableView = new TableView(this);
	outputAngleAxisTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	outputAngleAxisTableView->setModel(outputAngleAxisModel);
	outputAngleAxisTableView->verticalHeader()->setMinimumWidth(20);
	
	QVBoxLayout* outputAngleAxisLayout = new QVBoxLayout(outputAngleAxisGroupBox);
	outputAngleAxisLayout->addWidget(outputAngleAxisTableView);
	
	// quaternion
	
	QGroupBox* inputQuaternionGroupBox = new QGroupBox(this);
	inputQuaternionGroupBox->setFlat(true);
	inputQuaternionGroupBox->setTitle("Quaternion (Input)");
	gridLayout->addWidget(inputQuaternionGroupBox, gridLayout->rowCount(), 0);
	
	QuaternionModel* inputQuaternionModel = new QuaternionModel(this);
	inputQuaternionModel->quaternion = &this->inputQuaternion;
	
	TableView* inputQuaternionTableView = new TableView(this);
	inputQuaternionTableView->setModel(inputQuaternionModel);
	inputQuaternionTableView->verticalHeader()->setMinimumWidth(20);
	
	QVBoxLayout* inputQuaternionLayout = new QVBoxLayout(inputQuaternionGroupBox);
	inputQuaternionLayout->addWidget(inputQuaternionTableView);
	
	QObject::connect(
		inputQuaternionModel,
		SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
		this,
		SLOT(quaternionChanged(const QModelIndex&, const QModelIndex&))
	);
	
	QGroupBox* outputQuaternionGroupBox = new QGroupBox(this);
	outputQuaternionGroupBox->setFlat(true);
	outputQuaternionGroupBox->setTitle("Quaternion (Output)");
	gridLayout->addWidget(outputQuaternionGroupBox, gridLayout->rowCount() - 1, 1);
	
	outputQuaternionModel->quaternion = &this->outputQuaternion;
	
	TableView* outputQuaternionTableView = new TableView(this);
	outputQuaternionTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	outputQuaternionTableView->setModel(outputQuaternionModel);
	outputQuaternionTableView->verticalHeader()->setMinimumWidth(20);
	
	QVBoxLayout* outputQuaternionLayout = new QVBoxLayout(outputQuaternionGroupBox);
	outputQuaternionLayout->addWidget(outputQuaternionTableView);
	
	// eulerAngles
	
	QGroupBox* inputEulerAnglesGroupBox = new QGroupBox(this);
	inputEulerAnglesGroupBox->setFlat(true);
	inputEulerAnglesGroupBox->setTitle("Euler Angles (Input)");
	gridLayout->addWidget(inputEulerAnglesGroupBox, gridLayout->rowCount(), 0);
	
	inputEulerAnglesModel->eulerAngles = &this->inputEulerAngles;
	inputEulerAnglesModel->eulerAnglesRadians = &this->inputUnitRadians;
	inputEulerAnglesModel->eulerAxes = &this->inputEulerAxes;
	
	TableView* inputEulerAnglesTableView = new TableView(this);
	inputEulerAnglesTableView->setModel(inputEulerAnglesModel);
	inputEulerAnglesTableView->verticalHeader()->setMinimumWidth(20);
	
	QVBoxLayout* inputEulerAnglesLayout = new QVBoxLayout(inputEulerAnglesGroupBox);
	inputEulerAnglesLayout->addWidget(inputEulerAnglesTableView);
	
	QObject::connect(
		inputEulerAnglesModel,
		SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
		this,
		SLOT(eulerAnglesChanged(const QModelIndex&, const QModelIndex&))
	);
	
	QGroupBox* outputEulerAnglesGroupBox = new QGroupBox(this);
	outputEulerAnglesGroupBox->setFlat(true);
	outputEulerAnglesGroupBox->setTitle("Euler Angles (Output)");
	gridLayout->addWidget(outputEulerAnglesGroupBox, gridLayout->rowCount() - 1, 1);
	
	outputEulerAnglesModel->eulerAngles = &this->outputEulerAngles;
	outputEulerAnglesModel->eulerAnglesRadians = &this->outputUnitRadians;
	outputEulerAnglesModel->eulerAxes = &this->outputEulerAxes;
	
	TableView* outputEulerAnglesTableView = new TableView(this);
	outputEulerAnglesTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	outputEulerAnglesTableView->setModel(outputEulerAnglesModel);
	outputEulerAnglesTableView->verticalHeader()->setMinimumWidth(20);
	
	QVBoxLayout* outputEulerAnglesLayout = new QVBoxLayout(outputEulerAnglesGroupBox);
	outputEulerAnglesLayout->addWidget(outputEulerAnglesTableView);
}

MainWindow::~MainWindow()
{
	MainWindow::singleton = nullptr;
}

void
MainWindow::angleAxisChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
	this->outputAngleAxis = this->inputAngleAxis;
	this->outputQuaternion = this->inputAngleAxis;
	this->outputRotationMatrix = this->inputAngleAxis;
	
	this->outputEulerAngles = this->outputRotationMatrix.eulerAngles(this->outputEulerAxes[0], this->outputEulerAxes[1], this->outputEulerAxes[2]);
	
	this->outputAngleAxisModel->invalidate();
	this->outputEulerAnglesModel->invalidate();
	this->outputQuaternionModel->invalidate();
	this->outputRotationMatrixModel->invalidate();
}

void
MainWindow::eulerAnglesChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
	this->outputQuaternion = rl::math::AngleAxis(
		this->inputEulerAngles[0],
		rl::math::Vector3::Unit(this->inputEulerAxes[0])
	) * rl::math::AngleAxis(
		this->inputEulerAngles[1],
		rl::math::Vector3::Unit(this->inputEulerAxes[1])
	) * rl::math::AngleAxis(
		this->inputEulerAngles[2],
		rl::math::Vector3::Unit(this->inputEulerAxes[2])
	);
	
	this->outputAngleAxis = this->outputQuaternion;
	this->outputRotationMatrix = this->outputQuaternion;
	
	this->outputEulerAngles = this->outputRotationMatrix.eulerAngles(this->outputEulerAxes[0], this->outputEulerAxes[1], this->outputEulerAxes[2]);
	
	this->outputAngleAxisModel->invalidate();
	this->outputEulerAnglesModel->invalidate();
	this->outputQuaternionModel->invalidate();
	this->outputRotationMatrixModel->invalidate();
}

void
MainWindow::inputEulerAxesChanged(int index)
{
	if (index < 0 || index > 11)
	{
		return;
	}
	
	for (std::size_t i = 0; i < 3; ++i)
	{
		this->inputEulerAxes[i] = MainWindow::EULER_ANGLES[index][i];
	}
	
	this->inputEulerAnglesModel->invalidate();
	
	this->outputQuaternion = rl::math::AngleAxis(
		this->inputEulerAngles[0],
		rl::math::Vector3::Unit(this->inputEulerAxes[0])
	) * rl::math::AngleAxis(
		this->inputEulerAngles[1],
		rl::math::Vector3::Unit(this->inputEulerAxes[1])
	) * rl::math::AngleAxis(
		this->inputEulerAngles[2],
		rl::math::Vector3::Unit(this->inputEulerAxes[2])
	);
	
	this->outputAngleAxis = this->outputQuaternion;
	this->outputRotationMatrix = this->outputQuaternion;
	
	this->outputEulerAngles = this->outputRotationMatrix.eulerAngles(this->outputEulerAxes[0], this->outputEulerAxes[1], this->outputEulerAxes[2]);
	
	this->outputAngleAxisModel->invalidate();
	this->outputEulerAnglesModel->invalidate();
	this->outputQuaternionModel->invalidate();
	this->outputRotationMatrixModel->invalidate();
}

void
MainWindow::inputUnitChanged(int index)
{
	if (index < 0 || index > 1)
	{
		return;
	}
	
	switch (index)
	{
	case 0:
		this->inputUnitRadians = false;
		break;
	case 1:
		this->inputUnitRadians = true;
		break;
	default:
		break;
	}
	
	this->inputAngleAxisModel->invalidate();
	this->inputEulerAnglesModel->invalidate();
}

MainWindow*
MainWindow::instance()
{
	if (nullptr == MainWindow::singleton)
	{
		new MainWindow();
	}
	
	return MainWindow::singleton;
}

void
MainWindow::outputEulerAxesChanged(int index)
{
	if (index < 0 || index > 11)
	{
		return;
	}
	
	for (std::size_t i = 0; i < 3; ++i)
	{
		this->outputEulerAxes[i] = MainWindow::EULER_ANGLES[index][i];
	}
	
	this->outputEulerAngles = this->outputRotationMatrix.eulerAngles(this->outputEulerAxes[0], this->outputEulerAxes[1], this->outputEulerAxes[2]);
	
	this->outputEulerAnglesModel->invalidate();
}

void
MainWindow::outputUnitChanged(int index)
{
	if (index < 0 || index > 1)
	{
		return;
	}
	
	switch (index)
	{
	case 0:
		this->outputUnitRadians = false;
		break;
	case 1:
		this->outputUnitRadians = true;
		break;
	default:
		break;
	}
	
	this->outputAngleAxisModel->invalidate();
	this->outputEulerAnglesModel->invalidate();
}

void
MainWindow::quaternionChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
	this->outputAngleAxis = this->inputQuaternion;
	this->outputQuaternion = this->inputQuaternion;
	this->outputRotationMatrix = this->inputQuaternion;
	
	this->outputEulerAngles = this->outputRotationMatrix.eulerAngles(this->outputEulerAxes[0], this->outputEulerAxes[1], this->outputEulerAxes[2]);
	
	this->outputAngleAxisModel->invalidate();
	this->outputEulerAnglesModel->invalidate();
	this->outputQuaternionModel->invalidate();
	this->outputRotationMatrixModel->invalidate();
}

void
MainWindow::rotationMatrixChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
	this->outputAngleAxis = this->inputRotationMatrix;
	this->outputQuaternion = this->inputRotationMatrix;
	this->outputRotationMatrix = this->inputRotationMatrix;
	
	this->outputEulerAngles = this->outputRotationMatrix.eulerAngles(this->outputEulerAxes[0], this->outputEulerAxes[1], this->outputEulerAxes[2]);
	
	this->outputAngleAxisModel->invalidate();
	this->outputEulerAnglesModel->invalidate();
	this->outputQuaternionModel->invalidate();
	this->outputRotationMatrixModel->invalidate();
}

std::size_t MainWindow::EULER_ANGLES[12][3] = {
	{0, 1, 0},
	{0, 2, 0},
	{1, 0, 1},
	{1, 2, 1},
	{2, 0, 2},
	{2, 1, 2},
	{0, 1, 2},
	{0, 2, 1},
	{1, 0, 2},
	{1, 2, 0},
	{2, 0, 1},
	{2, 1, 0}
};
