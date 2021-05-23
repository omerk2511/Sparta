import Vue from "vue";
import VueRouter from "vue-router";
import vuetify from "./plugins/vuetify";
import TitleMixin from './mixins/TitleMixin'
import App from "./App.vue";
import Threats from "./components/Threats";
import Threat from "./components/Threat";
import Technique from "./components/Technique";
import Arsenal from "./components/Arsenal";
import About from "./components/About";

Vue.config.productionTip = false;

Vue.mixin(TitleMixin);

Vue.use(VueRouter);

const routes = [
 { path: '/', component: Threats },
 { path: '/threat/:id', component: Threat },
 { path: '/arsenal/:id', component: Technique },
 { path: '/arsenal', component: Arsenal },
 { path: '/about', component: About },
];

const router = new VueRouter({
  routes,
  mode: 'history',
});

new Vue({
  router,
  vuetify,
  render: (h) => h(App),
}).$mount("#app");
