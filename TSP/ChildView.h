
/*
 * 
 * website: N/A
 * contact: kataklinger@gmail.com
 *
 */

#pragma once

#include "TspAlgorithm.h"

// CChildView window

class CChildView : public CWnd,
	GaObserver
{

	DEFINE_SYNC_CLASS

// Construction
public:
	CChildView();

// Attributes
public:

	// Notifies the observer that new statistical information is available
	virtual void GACALL StatisticUpdate(const GaStatistics& statistics,
		const GaAlgorithm& algorithm);

	// Notifies observer that new best chromosome has found
	virtual void GACALL NewBestChromosome(const GaChromosome& newChromosome,
		const GaAlgorithm& algorithm);

	// Notifies observer that state of evolution (problem sloving) has changed.
	virtual void GACALL EvolutionStateChanged(GaAlgorithmState newState,
		const GaAlgorithm& algorithm) { }

private:

	GaCriticalSection _algControlSect;
	vector<const TspCity*> _bestChromosome;

	int _generation;
	float _fitness;

// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnStart();
	afx_msg void OnPause();
	afx_msg void OnStop();
};

