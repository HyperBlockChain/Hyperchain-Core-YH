if(/Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent)) {
	window.location = "view/home-mobile.html";
} else {
	window.location = "view/home.html";
}