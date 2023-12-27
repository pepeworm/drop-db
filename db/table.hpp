#pragma once

#include <string>
#include <vector>

enum class ColType { COL_TYPE_NULL, COL_TYPE_INT, COL_TYPE_DOUBLE, COL_TYPE_BLOB };

class ColInfo {
public:
	std::string name; // The column name
	ColType* type; // The column type
	bool req; // Whether the column requires a value (not `null`)

	/**
	 * @brief Define a table column
	 * @param name Name of the new table column
	 * @param type Type of the new table column
	 * @param req Whether the new table column requires a value (not `null`)
	 */
	ColInfo(std::string name, ColType* type, bool req) : name(name), type(type), req(req){};
};

class Table {
private:
	std::vector<ColInfo*> cols; // The column fields for the row

public:
};