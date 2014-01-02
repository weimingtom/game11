#include "CSV.h"

#include "npfile.h"

void CSV::load(const char* path) {
	nFile file;
	file.load(path);
	read(file.data());
}

void CSV::read(const char* data,char separater) {

	int pos = 0;
	while (true) {
		if (data[pos] == 0) {
			break;
		}

		Row* r = mRow.add();
		pos += r->read(data + pos,separater);
	}
};

char* CSV::data(int r,int c)const {
	Row::Cell* cl = cell(r,c);
	if (cl) {
		return cl->mValue.data();
	}
	else {
		return 0;
	}
};
int CSV::value(int r,int c) const {
	Row::Cell* cl = cell(r,c);
	if (cl) {
		return nString::calcChar(cl->mValue.data());
	}
	else {
		return 0;
	}
};
int CSV::line() const {
	int result = mRow.number();
	return result;
};

CSV::Row* CSV::row(int r) const{
	Row* result = mRow(r);
	return result;
};


CSV::Row::Cell* CSV::cell(int r, int c) const {
	return row(r)->cell(c);
};

CSV::Row::Cell* CSV::Row::cell(int c) const {
	Cell* result = mCell(c);
	return result;
};
int CSV::Row::line() const {
	return mCell.number();
};

int CSV::Row::read(const char* data,char separater) {
	int pos = 0;
	MyString tmp(256);	//‚±‚ê‚ªƒZƒ‹‚ÌÅ‘å’·‚³‚É‚È‚éB
	while(true) {
		char t = data[pos];
		if (t == 0) {
			break;
		}
		if (t == '\n') {
			pos += 1;
			break;
		}
		if (t == '\r') {
			++pos;
			continue;
		}
		if (t == separater) {
			++pos;

			Cell* c = mCell.add();
			c->mValue = tmp.data();

			tmp = "";
			continue;
		};

		if (t<0) {
			tmp << t;
			++pos;
			t = data[pos];
		};
		tmp << t;
		++pos;
	};

	if (!(tmp == "")) {
		Cell* c = mCell.add();
		c->mValue = tmp.data();
	}
	return pos;
}

