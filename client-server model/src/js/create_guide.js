
pathOpenId = "";
updateOpenId = "";
path = "/guides";

function toBind(){
  $("#create_guide_bto").click(function(event) {
      console.log(JSON.stringify({
        "name": $("#guide_new_name").val(),
        "email": $("#guide_new_email").val(),
        "cellular": $("#guide_new_cellular").val(),
      }))
    $.ajax({
      type: 'POST', // define the type of HTTP verb we want to use (POST for our form)
      url: path, // the url where we want to POST
      contentType: 'application/json',
      data: JSON.stringify({
        "name": $("#guide_new_name").val(),
        "email": $("#guide_new_email").val(),
        "cellular": $("#guide_new_cellular").val(),
      }),
      processData: true,
      encode: true,
      success: function (data, textStatus, jQxhr) {
        location.href = "/";
      },
      error: function (jqXhr, textStatus, errorThrown) {
        console.log(textStatus + jqXhr);
        console.log(errorThrown);
        alert("Error.Check that this Guide ID exist")
      }
    })
    event.preventDefault();
  });
 
  
}
// alert("Asdasda")

$(document).ready(toBind);

