/*
    This file is part of Cute Chess.

    Cute Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cute Chess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cute Chess.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QString>
#include <QIODevice>

#include "chessengine.h"
#include "chessboard/chess.h"

int ChessEngine::m_count = 0;

ChessEngine::ChessEngine(QIODevice* ioDevice,
                         Chess::Board* chessboard,
                         QObject* parent)
	: ChessPlayer(parent),
	  m_chessboard(chessboard),
	  m_notation(Chess::LongAlgebraic),
	  m_isReady(true),
	  m_initialized(false),
	  m_id(m_count++),
	  m_ioDevice(ioDevice)
{
	Q_ASSERT(m_ioDevice != 0);
	Q_ASSERT(m_chessboard != 0);
	Q_ASSERT(m_ioDevice->isOpen());
	
	QObject::connect(m_ioDevice, SIGNAL(readyRead()),
	                 this, SLOT(on_readyRead()));
}

ChessEngine::~ChessEngine()
{
	--m_count;
}

bool ChessEngine::isHuman() const
{
	return false;
}

void ChessEngine::write(const QString& data)
{
	if (!m_isReady)
	{
		m_writeBuffer.append(data);
		return;
	}

	Q_ASSERT(m_ioDevice->isWritable());
	emit debugMessage(QString(">") + name() + "(" + QString::number(m_id) +  "): " + data);

	m_ioDevice->write(data.toAscii() + "\n");
}

void ChessEngine::on_readyRead()
{
	while (m_ioDevice->canReadLine())
	{
		QString line = QString(m_ioDevice->readLine()).trimmed();
		emit debugMessage(QString("<") + name() + "(" + QString::number(m_id) +  "): " + line);
		
		parseLine(line);
	}
}

void ChessEngine::flushWriteBuffer()
{
	Q_ASSERT(m_isReady);
	
	if (m_writeBuffer.isEmpty())
		return;

	foreach (const QString& line, m_writeBuffer)
		write(line);
	m_writeBuffer.clear();
}

