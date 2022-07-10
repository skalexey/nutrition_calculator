
THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source $THIS_DIR/os.sh

if is_windows; then
	export nutrition_calculator_deps="${HOME}/Projects"
else
	export nutrition_calculator_deps="${HOME}/Projects"
fi