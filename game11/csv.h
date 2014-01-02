#ifndef INCLUDED_CSV_H
#define INCLUDED_CSV_H

#include "npstring.h"
#include "base.h"

class CSV{
	class Row{
	public:
		class Cell{
		public:
			MyString mValue;
		};
private:
		List<Cell> mCell;
public:

		int read(const char*,char separater = ',');
		Cell* cell(int i) const;
		int line() const;
	};

private:
	List<Row> mRow;
public:

	void load(const char* path);
	void read(const char*,char separater = ',');

	Row* row(int i) const;
	Row::Cell* cell(int r,int c) const;
	char* data(int r,int c) const;
	int value(int r,int c)const;
	int line() const;

};



#endif