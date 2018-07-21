beta() {
	cp subcore subcore-build-$(cat .version)
	echo "DONE: subcore-build-$(cat .version)"
}
