function mycarousel_initCallback(carousel) {

	$('#next').bind('click', function() {
        carousel.next();
        return false;
    });

    $('#prev').bind('click', function() {
        carousel.prev();
        return false;
    });
};

$(function(){
	
	$('.slider-content ul').jcarousel({
		auto: 0,
		wrap: "circular",
		scroll: 1,
		visible: 1,
		initCallback: mycarousel_initCallback,
        buttonNextHTML: null,
        buttonPrevHTML: null
	});
	
	 $('.blink').
	    focus(function() {
	        if(this.title==this.value) {
	            this.value = '';
	        }
	    }).
	    blur(function(){
	        if(this.value=='') {
	            this.value = this.title;
	        }
	    });
	
	
});
