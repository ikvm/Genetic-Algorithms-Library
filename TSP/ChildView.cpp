
/*
 * 
 * website: N/A
 * contact: kataklinger@gmail.com
 *
 */

#include "stdafx.h"
#include "TSP.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
	_generation = 0;
	_fitness = 0;

	TSP::GetInstance().GetAlgorithm()->SubscribeObserver( this );
}

CChildView::~CChildView() { }


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_START, &CChildView::OnStart)
	ON_COMMAND(ID_PAUSE, &CChildView::OnPause)
	ON_COMMAND(ID_STOP, &CChildView::OnStop)
END_MESSAGE_MAP()

// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this);
	
	dc.SetBkMode( TRANSPARENT );

	CFont font1;
	font1.CreateFont( 24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE,
		_T("Georgia") );

	dc.SelectObject( &font1 );

	CString s;
	s.Format( _T( "%.2f" ), _fitness );
	CString str = _T( "Lenght: " );
	dc.TextOutW( 10, 10, str + s );

	s.Format( _T( "%i" ), _generation );
	str = _T( "Generation: " );
	dc.TextOutW( 10, 35, str + s );

	LOCK_THIS_OBJECT( lock );

	if( _bestChromosome.size() )
	{
		CPen pen1( PS_SOLID, 2, RGB( 225, 0, 0 ) );
		dc.SelectObject( &pen1 );

		int fx, fy;
		for( int i = (int)_bestChromosome.size() - 1; i >= 0 ; i-- )
		{
			const TspCity* c = _bestChromosome[ i ];

			if( !c )
				continue;

			if( i < (int)_bestChromosome.size() - 1 )
				dc.LineTo( c->GetX(), c->GetY() );
			else
			{
				fx = c->GetX();
				fy = c->GetY();
			}

			dc.MoveTo( c->GetX(), c->GetY() );
		}

		dc.LineTo( fx, fy );
	}

	CPen pen2( PS_SOLID, 2, RGB( 0, 0, 245 ) );
	dc.SelectObject( &pen2 );

	CBrush brush1( RGB( 0, 0, 129 ) );
	dc.SelectObject( &brush1 );

	CFont font2;
	font2.CreateFont( 14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_DONTCARE,
		_T("Georgia") );

	dc.SelectObject( &font2 );

	TspCities::GetInstance().DrawCities( dc );
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	LOCK_THIS_OBJECT( lock );

	GaAlgorithmState state = TSP::GetInstance().GetAlgorithm()->GetState();
	if( !( state & GAS_STOPPED ) && state != GAS_UNINITIALIZED )
		MessageBox( _T( "You must stop the algorithm first!" ), _T( "Warining" ), MB_OK | MB_ICONWARNING );
	else
	{
		const TspCity* city = TspCities::GetInstance().GetCityByPoint( point.x, point.y );
		if( city )
		{
			if( MessageBox( _T( "Do you want to remove the city?" ), _T( "Remove the city?" ), MB_YESNO | MB_ICONQUESTION ) == IDYES )
			{
				TspCities::GetInstance().RemoveCity( city->GetID() );
				_bestChromosome.clear();
			}
		}
		else
		{
			TspCities::GetInstance().AddCity( L"city", point.x, point.y );
			_bestChromosome.clear();
		}
	}

	UNLOCK( lock );

	Invalidate();

	CWnd::OnLButtonDown(nFlags, point);
}

void CChildView::StatisticUpdate(const GaStatistics& statistics,
									const GaAlgorithm& algorithm)
{
	_generation = statistics.GetCurrentGeneration();

	if( _generation % 1000 == 0 )
		Invalidate();
}

void CChildView::NewBestChromosome(const GaChromosome& newChromosome,
									  const GaAlgorithm& algorithm)
{
	LOCK_THIS_OBJECT( lock );

	_bestChromosome.clear();
	_bestChromosome = dynamic_cast<const TspChromosome*>( &newChromosome )->GetCode();

	_fitness = newChromosome.GetFitness();

	UNLOCK( lock );

	Invalidate();
}

void CChildView::OnStart()
{
	LOCK( _algControlSect );

	bool resume = TSP::GetInstance().GetAlgorithm()->GetState() == GAS_PAUSED;
	TSP::GetInstance().GetAlgorithm()->StartSolving( resume );

	UNLOCK( _algControlSect );
}

void CChildView::OnPause()
{
	LOCK( _algControlSect );

	TSP::GetInstance().GetAlgorithm()->PauseSolving();

	UNLOCK( _algControlSect );
}

void CChildView::OnStop()
{
	LOCK( _algControlSect );

	TSP::GetInstance().GetAlgorithm()->StopSolving();

	UNLOCK( _algControlSect );
}
