
pathOpenId = "";
updateOpenId = "";
path = "/tours";

function toBind(){
  $.ajax({
    type: 'GET',
    url: '/guides',
    success: function (result) {
      toReturn = "<option class=selectGuide id=ID value=0>";
      toReturn += "</option>";
      result.forEach((guide,index) => {
        toReturn += "<option class=selectGuide id=" + guide._id + " value=" + index + ">";
        toReturn += " " + guide.name + ",";
        toReturn += " " + guide.email + ",";
        toReturn += " " + guide.cellular + "";
        toReturn += "</option>";
      })
      
      $("#selectList").html(toReturn);
      $('#selectList').change(function () {
        $('#guide_id').val($("#selectList option:selected").attr("id"))
      });
    },
    error: function (err) {
      console.log("err", err);
    }
  });
  $("#new_bto").click(function(event) {
    
  
    $.ajax({
      type: 'POST', // define the type of HTTP verb we want to use (POST for our form)
      url: path, // the url where we want to POST
      contentType: 'application/json',
      data: JSON.stringify({
        "name": $("#tour_name").val(),
        "start_date": $("#start_date").val(),
        "duration": parseInt($("#duration").val()),
        "price": parseFloat($("#price").val()),
        "guide": {
          "_id": $("#guide_id").val()
        },
        "path": [],
      }),
      processData: true,
      encode: true,
      success: function (data, textStatus, jQxhr) {
        location.href = "/";

      },
      error: function (jqXhr, textStatus, errorThrown) {
        console.log(textStatus + jqXhr);
        console.log(errorThrown);
        alert("Error.Make sure that all inputs are valid")
      }
    })
    event.preventDefault();
  });
 
  
}

$(document).ready(toBind);

