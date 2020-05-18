#include <Differentiator.h>
#include <iostream>

int main(int argc, char** argv) {
	Differentiator differentiator;
	UnorderedMap<String, String> variables;
	differentiator.Differentiate(argv[1], argv[2]).ToPDF(argv[3]);
	return 0;
}