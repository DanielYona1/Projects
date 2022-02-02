
pathOpenId = "";
updateOpenId = "";
path = "/tours";

var guide;
var options;
function showToursForGuide(event)
{
  var to_show = event.target.id.replace("guide_tours_","");
  var result = "";
  $("#div6").html("");
  $.ajax({      
    type: 'GET', 
    url: '/tours/',
    success: function (res) {
      if(res.filter(tour => tour.guide === to_show).length == 0)
      {
        result+= "<div>This guide hasn't published any tours yet.</div>";
      }
      else
      {
        result += ("<table class = \"display\" id = \"myTable3\"><tr> <th id = \"tourid\"> Tour's name </th> <th> Starting date </th> <th>Duration</th><th>Price</th><th>Paths</th>");
        var map1 = res.filter(tour => tour.guide === to_show).map(elm => {
          var toReturn = "";
          toReturn += "<div id=\"div_path_"+elm.name +"\"><tr><td>" + elm.name + "</td><td>" + elm.start_date.substring(0,10) + "</td><td>" + elm.duration + "</td><td>" + elm.price + "</td><td><ul>";
          
          elm.path.forEach(site => {
            toReturn += "<li> Site name:" + site.name + " <br>Site country: " + site.country + "</li>"
          })
          toReturn += "</ul></td></tr></div>"
          return toReturn;  

        });
        for(var i = 0; i < map1.length ; i++)
        {
          result += map1[i];
        }
      }
      $("#div6").html(result);
      $("#myTable3").DataTable();

      
    },
    error: function(err){
      console.log(err);
    }
  });
}
function displayGuides()
{
  $.ajax({
    type: 'GET',
    url: '/guides',
    success: function (result) {
      var toReturn = "<ul>";
      result.forEach(guide =>{
        toReturn += "<li>Name: "+guide.name+" <br>Email: "+guide.email+" <br>Cellular: "+guide.cellular+"</li>"
      })
      toReturn+="</ul>";
      $("#div4").html(toReturn);
    },
    error: function (err) {
      console.log("err", err);
    }
  });
}
function deleteSite(event)
{
  var tourID = (event.target.id.replace("del_site_",""));
  var params = tourID.split("_");
  var siteName = params[1];
  var countryName = params[2];
  $.ajax({      
    type: 'DELETE', 
    url: '/tours/'+params[0].replace(" ","")+"/"+siteName+"/"+countryName,
    success: function (res) {
      location.href = "/";
    },
    error: function(err){
      console.log(err);
    }
  });
}
function createPath(event){
  var create_to = event.target.id.replace("c_path_" , "")
  $.ajax({
    type: 'PUT', // define the type of HTTP verb we want to use (POST for our form)
    url: '/tours/site/'+create_to,
    // the url where we want to POST
    contentType: 'application/json',
    data: JSON.stringify({
      "path": {
        "name" : $("#site_input").val() ,
        "country": $("#country_input").val()
      }
    }),
    processData: false,            
       // dataType: 'json', // what type of data do we expect back from the server
    encode: true,
    success: function( data, textStatus, jQxhr ){
        location.href = "/";

    },
    error: function( jqXhr, textStatus, errorThrown ){
          console.log(errorThrown);
        }   
  })}
function addPath(event)
{
  var add_to = event.target.id.replace("addto_","");
  var toAppend = "<div><p><input id=\"site_input\" type =\"text\" placeholder = \"Enter touring site name\"></p>";
  toAppend += "<p><input id= \"country_input\" type =\"text\" placeholder = \"Enter touring site country\"></p>";
  toAppend += "<p><button id = \"c_path_" +add_to +"\">Create new path</button></p>";
  $("#div2").append(toAppend);
  var temp = "#c_path_" + add_to;
  $(temp).click(createPath);
}
function showPath(event)
{
  var to_show = event.target.id.replace("path_","");
  var result = "";
  $("#div2").html("");
  $.ajax({      
    type: 'GET', 
    url: '/tours/'+to_show,
    success: function (res) {
      if(res.path.length == 0)
      {
        result+= "<div>There're no paths for this tour yet</div>";
      }
      else
      {
        result += ("<table ><tr> <th id = \"tourid\"> Tour Site </th> <th> Country </th> <th></th>");
        var map1 = res.path.map((elm) => {
          return "<div id=\"div_path_"+elm.name +"\"><tr><td>" + elm.name + "</td><td>" + elm.country + "</td><td><button class = \"del_site\" name = \"" + elm.name + "\" id = \" del_site_" +to_show + "_"+elm.name+"_"+elm.country+"\">Delete Site</button></td></tr></div>"   
        });
        for(var i = 0; i < map1.length ; i++)
        {
          result += map1[i];
        }
      }
      $("#div2").html(result);
      $("#div2").append("<div><button class=\"bt_add_path\" id = \"addto_"+to_show+"\">Add a new path for tour: "+res.name+ "</button></div>");
      $(".bt_add_path").click(addPath);
      $(".del_site").click(deleteSite);
      // $("#myTable4").DataTable();

    },
    error: function(err){
      console.log(err);
    }
  });
}

function deleteTour(event){
  var to_del = event.target.id.replace("bt_","").split("_")[0]
  $.ajax({      
    type: 'DELETE', 
    url: '/tours/'+to_del,
    success: function (res) {
      location.href = "/";
    },
    error: function(err){
      console.log(err);
    }
  });
}
function submitGuideUpdate(event)
{

  $.ajax({
    
    type: 'put', // define the type of HTTP verb we want to use 
    url: '/guides/' + event.target.id.replace("guide_",""), // the url where we want to put.
    contentType: 'application/json',
    data: JSON.stringify(
      {
        "name": $("#guide_name").val(),
        "email": $("#guide_email").val(),
        "cellular": $("#guide_phone").val(),
      }
    ),
    processData: false,
    encode: true,
    success: function () {
      location.href = "/";
    },
    error: function (jqXhr, textStatus, errorThrown) {
      alert("Error to update this tour. make sure that all inputs are valid")
      console.log(jqXhr,textStatus,errorThrown)
    }
  });
}
function updateGuide(event){
  var params = event.target.id.replace("bt_update_","").split("_");
  var toReturn = "<div><h1>Update guide" + params[1] + "</h1><br>";
  toReturn += "<label for=\"guide_new_name\">Guide's name</label><br>"
  toReturn += "<input type=\"text\" name=\"guide_name\" id=\"guide_name\"  ><br><br>"
  toReturn += "<label for=\"guide_new_name\">Guide's email</label><br>"
  toReturn += "<input type=\"text\" name=\"guide_email\" id=\"guide_email\" placeholder=\"Insert new email\" ><br><br>"
  toReturn += "<label for=\"guide_new_name\">Cellular</label><br>"
  toReturn += "<input type=\"text\" name=\"guide_phone\" id=\"guide_phone\" placeholder=\"Insert your cellular\" ><br><br>"
  toReturn += "<button class = \"submitGuideUpdate\" id = \"guide_" + params[0]+ "\">UPDATE GUIDE</button>"
  $("#div5").html(toReturn);
  $(".submitGuideUpdate").click(submitGuideUpdate);
}
function submitUpdate(event)
{
  $.ajax({
    type: 'put', // define the type of HTTP verb we want to use 
    url: '/tours/' + event.target.id.replace("tour_",""), // the url where we want to put.
    contentType: 'application/json',
    data: JSON.stringify(
      {
        "start_date": $("#starting_date").val(),
        "duration": $("#duration").val(),
        "price": $("#price").val(),
        "guide": {
          "_id": $("#guide_id").val()
        },
      }
    ),
    processData: false,
    encode: true,
    success: function () {
      location.href = "/";
    },
    error: function (jqXhr, textStatus, errorThrown) {
      alert("Error to update this tour. make sure that all inputs are valid")
      console.log(jqXhr,textStatus,errorThrown)
    }
  });
}
function updateTour(event){
  var params = event.target.id.replace("bt_update_","").split("_");
  var toReturn = "<div><h1>Update tour" + params[1] + "</h1><br>";
  toReturn += "<label for=\"guide_new_name\">Starting Date</label><br>"
  toReturn += "<input type=\"date\" name=\"starting_date\" id=\"starting_date\"  ><br><br>"
  toReturn += "<label for=\"guide_new_name\">Duration</label><br>"
  toReturn += "<input type=\"text\" name=\"duration\" id=\"duration\" placeholder=\"Insert your tour's name\" ><br><br>"
  toReturn += "<label for=\"guide_new_name\">Price</label><br>"
  toReturn += "<input type=\"text\" name=\"price\" id=\"price\" placeholder=\"Insert your tour's name\" ><br><br>"
  toReturn += "<label for=\"guide_new_name\">Guides</label><br>"
  toReturn += "<select id=\"updateSelectList\"></select><br><br>"
  toReturn += "<input type=\"text\" name=\"guide_id\" id=\"guide_id\" placeholder=\"ID\" ><br><br> "
  toReturn += "<button class = \"submitUpdate\" id = \"tour_" + event.target.id.replace("bt_update_","")+ "\">UPDATE TOUR</button>"
  $.ajax({
    type: 'GET',
    url: '/guides',
    success: function (result) {
      options = "<option class=selectGuide id=ID value=0>";
      options += "</option>";
      result.forEach((guide,index) => {
        options += "<option class=selectGuide id=" + guide._id + " value=" + index + ">";
        options += " " + guide.name + ",";
        options += " " + guide.email + ",";
        options += " " + guide.cellular + "";
        options += "</option>";
      })
      $("#updateSelectList").html(options);
      $('#updateSelectList').change(function () {
        $('#guide_id').val($("#updateSelectList option:selected").attr("id"))
      });
      $(".submitUpdate").click(submitUpdate)
    },
    error: function (err) {
      console.log("err", err);
    }
  });
 
  $("#div2").html(toReturn)
}

function delGuide(event){
  var to_del = event.target.id.replace("del_","").split("_")[0]
  $.ajax({      
    type: 'DELETE', 
    url: '/guides/'+to_del,
    success: function (res) {
      location.href = "/";
    },
    error: function(err){
      console.log(err);
    }
  });
}
async function toBind(){
  $.ajax({      
    type: 'GET', 
    url: path,
    success: function (res) {
      buto = "<a href=\"http://localhost:3000/create_tour\"> <button>  create tour </button> </a><br></br>";
      buto2 = "<a href=\"http://localhost:3000/create_guide\"> <button>  create guide </button> </a><br></br>";
      result = "";
      result += ("<table class = \"display\" id = \"myTable\"><thead><tr> <th id = \"tourid\"> Tour name </th> <th> Strating Date </th> <th> Duration </th>");
      result += ("<th> Price </th> <th> Guide name </th> <th> Guide Email</th> <th> Guide Phone </th><th> Operations </th></tr></thead>");
      result += "<tbody>";
      var map1 = res.map((elm,idx) => {
        $.ajax({
          type: 'GET',
          url: '/guides/' + elm.guide +"/",
          success: function (result) {
            $("#g_name_"+elm._id).html(result.name);
            $("#g_email_"+elm._id).html(result.email);
            $("#g_cellular_"+elm._id).html(result.cellular);

          },
          error: function (err) {
            console.log("err", err);
          }
        });
        return "<div id=\"div_" +elm._id +"\"><tr>" +"<td id=\"" +idx +"\">" + elm.name+ "</td>"+"<td>" + elm.start_date.substring(0,10)+ "</td>"+"<td>" + elm.duration
        + "</td>"+"<td>" + elm.price  + "</td>"+"<td id=\"g_name_" +elm._id +"\"></td>" + "</td>"+"<td id=\"g_email_" +elm._id +"\"></td>" + "</td>"+"<td id=\"g_cellular_" +elm._id +"\"></td>" + "<td><button class=\"bt_update\" id=\""+"bt_update_" + elm._id+"_"+elm.name+"\">"+ "Update</button>"+"<button class=\"bt_delete\" id=\""+"bt_" + elm._id+"_"+ elm.name+"\">"+ "Delete</button>"+"<button class=\"bt_path\" id=\""+"path_" + elm._id+"\">"+ "Show Paths</button>"+"</td>"+"</tr></div><div id=\"update_"  + elm._id+ "\""
      });
      for(var i = 0; i < map1.length ; i++)
      {
        result+=map1[i];
      }
      // var bt_row = "<tr><td>"+buto+"</td>"+buto2+"</td>"+buto3+"</td></tr>"
      result += ("</tbody></table>");
      $("#div1").html(result);
      $(".bt_delete").click(deleteTour);
      $(".bt_update").click(updateTour);
      $(".bt_path").click(showPath);
      $('#myTable').DataTable();
      $("#div3").html(buto);
      $("#div3").append(buto2);
      $("#display_guides").click(displayGuides)
    },
    error: function(err){
      console.log(err);
    }
  });
  $.ajax({
    type: 'GET',
    url: '/guides',
    success: function (res) {
      toReturn = "";
      toReturn += ("<table class = \"display\" id = \"myTable2\"><thead><tr> <th id = \"tourid\"> Guide's name </th> <th> Guide's email </th> <th> Guide's cellular </th><th> Operations </th></tr></thead>");
      toReturn += "<tbody>";
      var temp = "";
      res.forEach((elm,idx) =>{
        toReturn += "<div id=\"div_" +elm._id +"\"><tr>" +"<td id=\"" +idx +"\">" + elm.name+ "</td>"+"<td>" + elm.email+ "</td>"+"<td>" + elm.cellular
        + "<td><button class=\"guide_update\" id=\""+"bt_update_" + elm._id+"_"+elm.name+"\">"+ "Update</button>"+"<button class=\"bt_tours\" id=\""+"guide_tours_" + elm._id+"\">"+ "Show Tours</button>"+"<button class=\"del_guide\" id=\""+"del_" + elm._id+"\">"+ "Delete Guide</button>"+"</td>"+"</tr>"
      })
      toReturn += ("</tbody></table>");
      $("#div4").html(toReturn);
      $(".guide_update").click(updateGuide);
      $(".bt_tours").click(showToursForGuide)
      $('#myTable2').DataTable();
      $(".del_guide").click(delGuide);

    },
    error: function (err) {
      console.log("err", err);
    }
  });

}


$(document).ready(toBind);
